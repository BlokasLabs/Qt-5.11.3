/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtQml module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qv4datacollector.h"
#include "qv4debugger.h"
#include "qv4debugjob.h"

#include <private/qv4script_p.h>
#include <private/qv4string_p.h>
#include <private/qv4objectiterator_p.h>
#include <private/qv4identifier_p.h>
#include <private/qv4runtime_p.h>

#include <private/qqmlcontext_p.h>
#include <private/qqmlengine_p.h>

#include <QtCore/qjsonarray.h>
#include <QtCore/qjsonobject.h>

QT_BEGIN_NAMESPACE

QV4::CppStackFrame *QV4DataCollector::findFrame(int frame)
{
    QV4::CppStackFrame *f = engine()->currentStackFrame;
    while (f && frame) {
        --frame;
        f = f->parent;
    }
    return f;
}

QV4::Heap::ExecutionContext *QV4DataCollector::findContext(int frame)
{
    QV4::CppStackFrame *f = findFrame(frame);

    return f ? f->context()->d() : nullptr;
}

QV4::Heap::ExecutionContext *QV4DataCollector::findScope(QV4::Heap::ExecutionContext *ctx, int scope)
{
    for (; scope > 0 && ctx; --scope)
        ctx = ctx->outer;

    return ctx;
}

QVector<QV4::Heap::ExecutionContext::ContextType> QV4DataCollector::getScopeTypes(int frame)
{
    QVector<QV4::Heap::ExecutionContext::ContextType> types;

    QV4::Heap::ExecutionContext *it = findFrame(frame)->context()->d();

    for (; it; it = it->outer)
        types.append(QV4::Heap::ExecutionContext::ContextType(it->type));

    return types;
}

int QV4DataCollector::encodeScopeType(QV4::Heap::ExecutionContext::ContextType scopeType)
{
    switch (scopeType) {
    case QV4::Heap::ExecutionContext::Type_GlobalContext:
        return 0;
    case QV4::Heap::ExecutionContext::Type_CatchContext:
        return 4;
    case QV4::Heap::ExecutionContext::Type_WithContext:
        return 2;
    case QV4::Heap::ExecutionContext::Type_CallContext:
        return 1;
    case QV4::Heap::ExecutionContext::Type_QmlContext:
        return 3;
    default:
        return -1;
    }
}

QV4DataCollector::QV4DataCollector(QV4::ExecutionEngine *engine)
    : m_engine(engine), m_namesAsObjects(true), m_redundantRefs(true)
{
    m_values.set(engine, engine->newArrayObject());
}

// TODO: Directly call addRef() once we don't need to support redundantRefs anymore
QV4DataCollector::Ref QV4DataCollector::collect(const QV4::ScopedValue &value)
{
    Ref ref = addRef(value);
    if (m_redundantRefs)
        m_collectedRefs.append(ref);
    return ref;
}

const QV4::Object *collectProperty(const QV4::ScopedValue &value, QV4::ExecutionEngine *engine,
                                   QJsonObject &dict)
{
    QV4::Scope scope(engine);
    QV4::ScopedValue typeString(scope, QV4::Runtime::method_typeofValue(engine, value));
    dict.insert(QStringLiteral("type"), typeString->toQStringNoThrow());

    const QLatin1String valueKey("value");
    switch (value->type()) {
    case QV4::Value::Empty_Type:
        Q_ASSERT(!"empty Value encountered");
        return nullptr;
    case QV4::Value::Undefined_Type:
        dict.insert(valueKey, QJsonValue::Undefined);
        return nullptr;
    case QV4::Value::Null_Type:
        dict.insert(valueKey, QJsonValue::Null);
        return nullptr;
    case QV4::Value::Boolean_Type:
        dict.insert(valueKey, value->booleanValue());
        return nullptr;
    case QV4::Value::Managed_Type:
        if (const QV4::String *s = value->as<QV4::String>()) {
            dict.insert(valueKey, s->toQString());
        } else if (const QV4::ArrayObject *a = value->as<QV4::ArrayObject>()) {
            // size of an array is number of its numerical properties; We don't consider free form
            // object properties here.
            dict.insert(valueKey, qint64(a->getLength()));
            return a;
        } else if (const QV4::Object *o = value->as<QV4::Object>()) {
            int numProperties = 0;
            QV4::ObjectIterator it(scope, o, QV4::ObjectIterator::EnumerableOnly);
            QV4::PropertyAttributes attrs;
            uint index;
            QV4::ScopedProperty p(scope);
            QV4::ScopedString name(scope);
            while (true) {
                it.next(name.getRef(), &index, p, &attrs);
                if (attrs.isEmpty())
                    break;
                else
                    ++numProperties;
            }
            dict.insert(valueKey, numProperties);
            return o;
        } else {
            Q_UNREACHABLE();
        }
        return nullptr;
    case QV4::Value::Integer_Type:
        dict.insert(valueKey, value->integerValue());
        return nullptr;
    default: {// double
        const double val = value->doubleValue();
        if (qIsFinite(val))
            dict.insert(valueKey, val);
        else if (qIsNaN(val))
            dict.insert(valueKey, QStringLiteral("NaN"));
        else if (val < 0)
            dict.insert(valueKey, QStringLiteral("-Infinity"));
        else
            dict.insert(valueKey, QStringLiteral("Infinity"));
        return nullptr;
    }
    }
}

QJsonObject QV4DataCollector::lookupRef(Ref ref, bool deep)
{
    QJsonObject dict;

    if (m_namesAsObjects) {
        if (lookupSpecialRef(ref, &dict))
            return dict;
    }

    if (m_redundantRefs)
        deep = true;

    dict.insert(QStringLiteral("handle"), qint64(ref));
    QV4::Scope scope(engine());
    QV4::ScopedValue value(scope, getValue(ref));

    const QV4::Object *object = collectProperty(value, engine(), dict);
    if (deep && object)
        dict.insert(QStringLiteral("properties"), collectProperties(object));

    return dict;
}

// TODO: Drop this method once we don't need to support namesAsObjects anymore
QV4DataCollector::Ref QV4DataCollector::addFunctionRef(const QString &functionName)
{
    Q_ASSERT(m_namesAsObjects);
    Ref ref = addRef(QV4::Primitive::emptyValue(), false);

    QJsonObject dict;
    dict.insert(QStringLiteral("handle"), qint64(ref));
    dict.insert(QStringLiteral("type"), QStringLiteral("function"));
    dict.insert(QStringLiteral("name"), functionName);
    m_specialRefs.insert(ref, dict);
    m_collectedRefs.append(ref);

    return ref;
}

// TODO: Drop this method once we don't need to support namesAsObjects anymore
QV4DataCollector::Ref QV4DataCollector::addScriptRef(const QString &scriptName)
{
    Q_ASSERT(m_namesAsObjects);
    Ref ref = addRef(QV4::Primitive::emptyValue(), false);

    QJsonObject dict;
    dict.insert(QStringLiteral("handle"), qint64(ref));
    dict.insert(QStringLiteral("type"), QStringLiteral("script"));
    dict.insert(QStringLiteral("name"), scriptName);
    m_specialRefs.insert(ref, dict);
    m_collectedRefs.append(ref);

    return ref;
}

bool QV4DataCollector::isValidRef(QV4DataCollector::Ref ref) const
{
    QV4::Scope scope(engine());
    QV4::ScopedObject array(scope, m_values.value());
    return ref < array->getLength();
}

bool QV4DataCollector::collectScope(QJsonObject *dict, int frameNr, int scopeNr)
{
    QV4::Scope scope(engine());

    QV4::Scoped<QV4::ExecutionContext> ctxt(scope, findScope(findContext(frameNr), scopeNr));
    if (!ctxt)
        return false;

    QV4::ScopedObject scopeObject(scope, engine()->newObject());
    if (ctxt->d()->type == QV4::Heap::ExecutionContext::Type_CallContext) {
        QStringList names;
        Refs collectedRefs;

        QV4::ScopedValue v(scope);
        QV4::InternalClass *ic = ctxt->internalClass();
        for (uint i = 0; i < ic->size; ++i) {
            QString name = ic->nameMap[i]->string;
            names.append(name);
            v = static_cast<QV4::Heap::CallContext *>(ctxt->d())->locals[i];
            collectedRefs.append(collect(v));
        }

        Q_ASSERT(names.size() == collectedRefs.size());
        QV4::ScopedString propName(scope);
        for (int i = 0, ei = collectedRefs.size(); i != ei; ++i) {
            propName = engine()->newString(names.at(i));
            scopeObject->put(propName, (v = getValue(collectedRefs.at(i))));
        }
    }

    Ref scopeObjectRef = addRef(scopeObject);
    if (m_redundantRefs) {
        dict->insert(QStringLiteral("ref"), qint64(scopeObjectRef));
        m_collectedRefs.append(scopeObjectRef);
    } else {
        *dict = lookupRef(scopeObjectRef, true);
    }

    return true;
}

QJsonObject toRef(QV4DataCollector::Ref ref) {
    QJsonObject dict;
    dict.insert(QStringLiteral("ref"), qint64(ref));
    return dict;
}

QJsonObject QV4DataCollector::buildFrame(const QV4::StackFrame &stackFrame, int frameNr)
{
    QJsonObject frame;
    frame[QLatin1String("index")] = frameNr;
    frame[QLatin1String("debuggerFrame")] = false;
    if (m_namesAsObjects) {
        frame[QLatin1String("func")] = toRef(addFunctionRef(stackFrame.function));
        frame[QLatin1String("script")] = toRef(addScriptRef(stackFrame.source));
    } else {
        frame[QLatin1String("func")] = stackFrame.function;
        frame[QLatin1String("script")] = stackFrame.source;
    }
    frame[QLatin1String("line")] = stackFrame.line - 1;
    if (stackFrame.column >= 0)
        frame[QLatin1String("column")] = stackFrame.column;

    QJsonArray scopes;
    QV4::Scope scope(engine());
    QV4::ScopedContext ctxt(scope, findContext(frameNr));
    while (ctxt) {
        if (QV4::CallContext *cCtxt = ctxt->asCallContext()) {
            if (cCtxt->d()->activation)
                break;
        }
        ctxt = ctxt->d()->outer;
    }

    if (ctxt) {
        QV4::ScopedValue o(scope, ctxt->d()->activation);
        frame[QLatin1String("receiver")] = toRef(collect(o));
    }

    // Only type and index are used by Qt Creator, so we keep it easy:
    QVector<QV4::Heap::ExecutionContext::ContextType> scopeTypes = getScopeTypes(frameNr);
    for (int i = 0, ei = scopeTypes.count(); i != ei; ++i) {
        int type = encodeScopeType(scopeTypes[i]);
        if (type == -1)
            continue;

        QJsonObject scope;
        scope[QLatin1String("index")] = i;
        scope[QLatin1String("type")] = type;
        scopes.push_back(scope);
    }

    frame[QLatin1String("scopes")] = scopes;

    return frame;
}

// TODO: Drop this method once we don't need to support redundantRefs anymore
QJsonArray QV4DataCollector::flushCollectedRefs()
{
    Q_ASSERT(m_redundantRefs);
    QJsonArray refs;
    std::sort(m_collectedRefs.begin(), m_collectedRefs.end());
    for (int i = 0, ei = m_collectedRefs.size(); i != ei; ++i) {
        QV4DataCollector::Ref ref = m_collectedRefs.at(i);
        if (i > 0 && ref == m_collectedRefs.at(i - 1))
            continue;
        refs.append(lookupRef(ref, true));
    }

    m_collectedRefs.clear();
    return refs;
}

void QV4DataCollector::clear()
{
    m_values.set(engine(), engine()->newArrayObject());
    m_collectedRefs.clear();
    m_specialRefs.clear();
    m_namesAsObjects = true;
    m_redundantRefs = true;
}

QV4DataCollector::Ref QV4DataCollector::addRef(QV4::Value value, bool deduplicate)
{
    class ExceptionStateSaver
    {
        quint8 *hasExceptionLoc;
        quint8 hadException;

    public:
        ExceptionStateSaver(QV4::ExecutionEngine *engine)
            : hasExceptionLoc(&engine->hasException)
            , hadException(false)
        { std::swap(*hasExceptionLoc, hadException); }

        ~ExceptionStateSaver()
        { std::swap(*hasExceptionLoc, hadException); }
    };

    // if we wouldn't do this, the putIndexed won't work.
    ExceptionStateSaver resetExceptionState(engine());
    QV4::Scope scope(engine());
    QV4::ScopedObject array(scope, m_values.value());
    if (deduplicate) {
        for (Ref i = 0; i < array->getLength(); ++i) {
            if (array->getIndexed(i) == value.rawValue() && !m_specialRefs.contains(i))
                return i;
        }
    }
    Ref ref = array->getLength();
    array->putIndexed(ref, value);
    Q_ASSERT(array->getLength() - 1 == ref);
    return ref;
}

QV4::ReturnedValue QV4DataCollector::getValue(Ref ref)
{
    QV4::Scope scope(engine());
    QV4::ScopedObject array(scope, m_values.value());
    Q_ASSERT(ref < array->getLength());
    return array->getIndexed(ref, nullptr);
}

// TODO: Drop this method once we don't need to support namesAsObjects anymore
bool QV4DataCollector::lookupSpecialRef(Ref ref, QJsonObject *dict)
{
    Q_ASSERT(m_namesAsObjects);
    SpecialRefs::const_iterator it = m_specialRefs.constFind(ref);
    if (it == m_specialRefs.cend())
        return false;

    *dict = it.value();
    return true;
}

QJsonArray QV4DataCollector::collectProperties(const QV4::Object *object)
{
    QJsonArray res;

    QV4::Scope scope(engine());
    QV4::ObjectIterator it(scope, object, QV4::ObjectIterator::EnumerableOnly);
    QV4::ScopedValue name(scope);
    QV4::ScopedValue value(scope);
    while (true) {
        QV4::Value v;
        name = it.nextPropertyNameAsString(&v);
        if (name->isNull())
            break;
        QString key = name->toQStringNoThrow();
        value = v;
        res.append(collectAsJson(key, value));
    }

    return res;
}

QJsonObject QV4DataCollector::collectAsJson(const QString &name, const QV4::ScopedValue &value)
{
    QJsonObject dict;
    if (!name.isNull())
        dict.insert(QStringLiteral("name"), name);
    if (value->isManaged() && !value->isString()) {
        Ref ref = addRef(value);
        dict.insert(QStringLiteral("ref"), qint64(ref));
        if (m_redundantRefs)
            m_collectedRefs.append(ref);
    }

    collectProperty(value, engine(), dict);
    return dict;
}

QT_END_NAMESPACE
