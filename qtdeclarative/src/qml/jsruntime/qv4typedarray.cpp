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
#include "qv4typedarray_p.h"
#include "qv4arraybuffer_p.h"
#include "qv4string_p.h"
#include "qv4jscall_p.h"

#include <cmath>

using namespace QV4;

DEFINE_OBJECT_VTABLE(TypedArrayCtor);
DEFINE_OBJECT_VTABLE(TypedArrayPrototype);
DEFINE_OBJECT_VTABLE(TypedArray);

Q_STATIC_ASSERT((int)ExecutionEngine::NTypedArrayTypes == (int)Heap::TypedArray::NTypes);

ReturnedValue Int8ArrayRead(const char *data, int index)
{
    return Encode((int)(signed char)data[index]);
}

void Int8ArrayWrite(ExecutionEngine *e, char *data, int index, const Value &value)
{
    signed char v = (signed char)value.toUInt32();
    if (e->hasException)
        return;
    data[index] = v;
}

ReturnedValue UInt8ArrayRead(const char *data, int index)
{
    return Encode((int)(unsigned char)data[index]);
}

void UInt8ArrayWrite(ExecutionEngine *e, char *data, int index, const Value &value)
{
    unsigned char v = (unsigned char)value.toUInt32();
    if (e->hasException)
        return;
    data[index] = v;
}

void UInt8ClampedArrayWrite(ExecutionEngine *e, char *data, int index, const Value &value)
{
    if (value.isInteger()) {
        data[index] = (char)(unsigned char)qBound(0, value.integerValue(), 255);
        return;
    }
    double d = value.toNumber();
    if (e->hasException)
        return;
    // ### is there a way to optimise this?
    if (d <= 0 || std::isnan(d)) {
        data[index] = 0;
        return;
    }
    if (d >= 255) {
        data[index] = (char)(255);
        return;
    }
    double f = std::floor(d);
    if (f + 0.5 < d) {
        data[index] = (unsigned char)(f + 1);
        return;
    }
    if (d < f + 0.5) {
        data[index] = (unsigned char)(f);
        return;
    }
    if (int(f) % 2) {
        // odd number
        data[index] = (unsigned char)(f + 1);
        return;
    }
    data[index] = (unsigned char)(f);
}

ReturnedValue Int16ArrayRead(const char *data, int index)
{
    return Encode((int)*(const short *)(data + index));
}

void Int16ArrayWrite(ExecutionEngine *e, char *data, int index, const Value &value)
{
    short v = (short)value.toInt32();
    if (e->hasException)
        return;
    *(short *)(data + index) = v;
}

ReturnedValue UInt16ArrayRead(const char *data, int index)
{
    return Encode((int)*(const unsigned short *)(data + index));
}

void UInt16ArrayWrite(ExecutionEngine *e, char *data, int index, const Value &value)
{
    unsigned short v = (unsigned short)value.toInt32();
    if (e->hasException)
        return;
    *(unsigned short *)(data + index) = v;
}

ReturnedValue Int32ArrayRead(const char *data, int index)
{
    return Encode(*(const int *)(data + index));
}

void Int32ArrayWrite(ExecutionEngine *e, char *data, int index, const Value &value)
{
    int v = (int)value.toInt32();
    if (e->hasException)
        return;
    *(int *)(data + index) = v;
}

ReturnedValue UInt32ArrayRead(const char *data, int index)
{
    return Encode(*(const unsigned int *)(data + index));
}

void UInt32ArrayWrite(ExecutionEngine *e, char *data, int index, const Value &value)
{
    unsigned int v = (unsigned int)value.toUInt32();
    if (e->hasException)
        return;
    *(unsigned int *)(data + index) = v;
}

ReturnedValue Float32ArrayRead(const char *data, int index)
{
    return Encode(*(const float *)(data + index));
}

void Float32ArrayWrite(ExecutionEngine *e, char *data, int index, const Value &value)
{
    float v = value.toNumber();
    if (e->hasException)
        return;
    *(float *)(data + index) = v;
}

ReturnedValue Float64ArrayRead(const char *data, int index)
{
    return Encode(*(const double *)(data + index));
}

void Float64ArrayWrite(ExecutionEngine *e, char *data, int index, const Value &value)
{
    double v = value.toNumber();
    if (e->hasException)
        return;
    *(double *)(data + index) = v;
}

const TypedArrayOperations operations[Heap::TypedArray::NTypes] = {
    { 1, "Int8Array", Int8ArrayRead, Int8ArrayWrite },
    { 1, "Uint8Array", UInt8ArrayRead, UInt8ArrayWrite },
    { 1, "Uint8ClampedArray", UInt8ArrayRead, UInt8ClampedArrayWrite },
    { 2, "Int16Array", Int16ArrayRead, Int16ArrayWrite },
    { 2, "Uint16Array", UInt16ArrayRead, UInt16ArrayWrite },
    { 4, "Int32Array", Int32ArrayRead, Int32ArrayWrite },
    { 4, "Uint32Array", UInt32ArrayRead, UInt32ArrayWrite },
    { 4, "Float32Array", Float32ArrayRead, Float32ArrayWrite },
    { 8, "Float64Array", Float64ArrayRead, Float64ArrayWrite },
};


void Heap::TypedArrayCtor::init(QV4::ExecutionContext *scope, TypedArray::Type t)
{
    Heap::FunctionObject::init(scope, QLatin1String(operations[t].name));
    type = t;
}

ReturnedValue TypedArrayCtor::callAsConstructor(const FunctionObject *f, const Value *argv, int argc)
{
    Scope scope(f->engine());
    const TypedArrayCtor *that = static_cast<const TypedArrayCtor *>(f);

    if (!argc || !argv[0].isObject()) {
        // ECMA 6 22.2.1.1
        double l = argc ? argv[0].toNumber() : 0;
        if (scope.engine->hasException)
            return Encode::undefined();
        uint len = (uint)l;
        if (l != len)
            scope.engine->throwRangeError(QStringLiteral("Non integer length for typed array."));
        uint byteLength = len * operations[that->d()->type].bytesPerElement;
        Scoped<ArrayBuffer> buffer(scope, scope.engine->newArrayBuffer(byteLength));
        if (scope.engine->hasException)
            return Encode::undefined();

        Scoped<TypedArray> array(scope, TypedArray::create(scope.engine, that->d()->type));
        array->d()->buffer.set(scope.engine, buffer->d());
        array->d()->byteLength = byteLength;
        array->d()->byteOffset = 0;

        return array.asReturnedValue();
    }
    Scoped<TypedArray> typedArray(scope, argc ? argv[0] : Primitive::undefinedValue());
    if (!!typedArray) {
        // ECMA 6 22.2.1.2
        Scoped<ArrayBuffer> buffer(scope, typedArray->d()->buffer);
        uint srcElementSize = typedArray->d()->type->bytesPerElement;
        uint destElementSize = operations[that->d()->type].bytesPerElement;
        uint byteLength = typedArray->d()->byteLength;
        uint destByteLength = byteLength*destElementSize/srcElementSize;

        Scoped<ArrayBuffer> newBuffer(scope, scope.engine->newArrayBuffer(destByteLength));
        if (scope.engine->hasException)
            return Encode::undefined();

        Scoped<TypedArray> array(scope, TypedArray::create(scope.engine, that->d()->type));
        array->d()->buffer.set(scope.engine, newBuffer->d());
        array->d()->byteLength = destByteLength;
        array->d()->byteOffset = 0;

        const char *src = buffer->d()->data->data() + typedArray->d()->byteOffset;
        char *dest = newBuffer->d()->data->data();

        // check if src and new type have the same size. In that case we can simply memcpy the data
        if (srcElementSize == destElementSize) {
            memcpy(dest, src, byteLength);
        } else {
            // not same size, we need to loop
            uint l = typedArray->length();
            TypedArrayRead read = typedArray->d()->type->read;
            TypedArrayWrite write =array->d()->type->write;
            for (uint i = 0; i < l; ++i) {
                Primitive val;
                val.setRawValue(read(src, i*srcElementSize));
                write(scope.engine, dest, i*destElementSize, val);
            }
        }

        return array.asReturnedValue();
    }
    Scoped<ArrayBuffer> buffer(scope, argc ? argv[0] : Primitive::undefinedValue());
    if (!!buffer) {
        // ECMA 6 22.2.1.4

        double dbyteOffset = argc > 1 ? argv[1].toInteger() : 0;
        uint byteOffset = (uint)dbyteOffset;
        uint elementSize = operations[that->d()->type].bytesPerElement;
        if (dbyteOffset < 0 || (byteOffset % elementSize) || dbyteOffset > buffer->byteLength())
            return scope.engine->throwRangeError(QStringLiteral("new TypedArray: invalid byteOffset"));

        uint byteLength;
        if (argc < 3 || argv[2].isUndefined()) {
            byteLength = buffer->byteLength() - byteOffset;
            if (buffer->byteLength() < byteOffset || byteLength % elementSize)
                return scope.engine->throwRangeError(QStringLiteral("new TypedArray: invalid length"));
        } else {
            double l = qBound(0., argv[2].toInteger(), (double)UINT_MAX);
            if (scope.engine->hasException)
                return Encode::undefined();
            l *= elementSize;
            if (buffer->byteLength() - byteOffset < l)
                return scope.engine->throwRangeError(QStringLiteral("new TypedArray: invalid length"));
            byteLength = (uint)l;
        }

        Scoped<TypedArray> array(scope, TypedArray::create(scope.engine, that->d()->type));
        array->d()->buffer.set(scope.engine, buffer->d());
        array->d()->byteLength = byteLength;
        array->d()->byteOffset = byteOffset;
        return array.asReturnedValue();
    }

    // ECMA 6 22.2.1.3

    ScopedObject o(scope, argc ? argv[0] : Primitive::undefinedValue());
    uint l = (uint) qBound(0., ScopedValue(scope, o->get(scope.engine->id_length()))->toInteger(), (double)UINT_MAX);
    if (scope.engine->hasException)
        return scope.engine->throwTypeError();

    uint elementSize = operations[that->d()->type].bytesPerElement;
    Scoped<ArrayBuffer> newBuffer(scope, scope.engine->newArrayBuffer(l * elementSize));
    if (scope.engine->hasException)
        return Encode::undefined();

    Scoped<TypedArray> array(scope, TypedArray::create(scope.engine, that->d()->type));
    array->d()->buffer.set(scope.engine, newBuffer->d());
    array->d()->byteLength = l * elementSize;
    array->d()->byteOffset = 0;

    uint idx = 0;
    char *b = newBuffer->d()->data->data();
    ScopedValue val(scope);
    while (idx < l) {
        val = o->getIndexed(idx);
        array->d()->type->write(scope.engine, b, 0, val);
        if (scope.engine->hasException)
            return Encode::undefined();
        ++idx;
        b += elementSize;
    }


    return array.asReturnedValue();
}

ReturnedValue TypedArrayCtor::call(const FunctionObject *f, const Value *, const Value *argv, int argc)
{
    return callAsConstructor(f, argv, argc);
}

void Heap::TypedArray::init(Type t)
{
    Object::init();
    type = operations + t;
    arrayType = t;
}

Heap::TypedArray *TypedArray::create(ExecutionEngine *e, Heap::TypedArray::Type t)
{
    QV4::InternalClass *ic = e->internalClasses[EngineBase::Class_Empty]->changeVTable(staticVTable());
    ic = ic->changePrototype(e->typedArrayPrototype[t].d());
    return e->memoryManager->allocObject<TypedArray>(ic, e->typedArrayPrototype + t, t);
}

ReturnedValue TypedArray::getIndexed(const Managed *m, uint index, bool *hasProperty)
{
    Scope scope(static_cast<const Object *>(m)->engine());
    Scoped<TypedArray> a(scope, static_cast<const TypedArray *>(m));

    uint bytesPerElement = a->d()->type->bytesPerElement;
    uint byteOffset = a->d()->byteOffset + index * bytesPerElement;
    if (byteOffset + bytesPerElement > (uint)a->d()->buffer->byteLength()) {
        if (hasProperty)
            *hasProperty = false;
        return Encode::undefined();
    }
    if (hasProperty)
        *hasProperty = true;
    return a->d()->type->read(a->d()->buffer->data->data(), byteOffset);
}

bool TypedArray::putIndexed(Managed *m, uint index, const Value &value)
{
    ExecutionEngine *v4 = static_cast<Object *>(m)->engine();
    if (v4->hasException)
        return false;

    Scope scope(v4);
    Scoped<TypedArray> a(scope, static_cast<TypedArray *>(m));

    uint bytesPerElement = a->d()->type->bytesPerElement;
    uint byteOffset = a->d()->byteOffset + index * bytesPerElement;
    if (byteOffset + bytesPerElement > (uint)a->d()->buffer->byteLength())
        return false;

    a->d()->type->write(scope.engine, a->d()->buffer->data->data(), byteOffset, value);
    return true;
}

void TypedArrayPrototype::init(ExecutionEngine *engine, TypedArrayCtor *ctor)
{
    Scope scope(engine);
    ScopedObject o(scope);
    ctor->defineReadonlyProperty(engine->id_length(), Primitive::fromInt32(3));
    ctor->defineReadonlyProperty(engine->id_prototype(), (o = this));
    ctor->defineReadonlyProperty(QStringLiteral("BYTES_PER_ELEMENT"), Primitive::fromInt32(operations[ctor->d()->type].bytesPerElement));
    defineDefaultProperty(engine->id_constructor(), (o = ctor));
    defineAccessorProperty(QStringLiteral("buffer"), method_get_buffer, nullptr);
    defineAccessorProperty(QStringLiteral("byteLength"), method_get_byteLength, nullptr);
    defineAccessorProperty(QStringLiteral("byteOffset"), method_get_byteOffset, nullptr);
    defineAccessorProperty(QStringLiteral("length"), method_get_length, nullptr);
    defineReadonlyProperty(QStringLiteral("BYTES_PER_ELEMENT"), Primitive::fromInt32(operations[ctor->d()->type].bytesPerElement));

    defineDefaultProperty(QStringLiteral("set"), method_set, 1);
    defineDefaultProperty(QStringLiteral("subarray"), method_subarray, 0);
}

ReturnedValue TypedArrayPrototype::method_get_buffer(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    ExecutionEngine *v4 = b->engine();
    const TypedArray *v = thisObject->as<TypedArray>();
    if (!v)
        return v4->throwTypeError();

    return v->d()->buffer->asReturnedValue();
}

ReturnedValue TypedArrayPrototype::method_get_byteLength(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    ExecutionEngine *v4 = b->engine();
    const TypedArray *v = thisObject->as<TypedArray>();
    if (!v)
        return v4->throwTypeError();

    return Encode(v->d()->byteLength);
}

ReturnedValue TypedArrayPrototype::method_get_byteOffset(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    ExecutionEngine *v4 = b->engine();
    const TypedArray *v = thisObject->as<TypedArray>();
    if (!v)
        return v4->throwTypeError();

    return Encode(v->d()->byteOffset);
}

ReturnedValue TypedArrayPrototype::method_get_length(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    ExecutionEngine *v4 = b->engine();
    const TypedArray *v = thisObject->as<TypedArray>();
    if (!v)
        return v4->throwTypeError();

    return Encode(v->d()->byteLength/v->d()->type->bytesPerElement);
}

ReturnedValue TypedArrayPrototype::method_set(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    Scoped<TypedArray> a(scope, *thisObject);
    if (!a)
        return scope.engine->throwTypeError();
    Scoped<ArrayBuffer> buffer(scope, a->d()->buffer);
    if (!buffer)
        scope.engine->throwTypeError();

    double doffset = argc >= 2 ? argv[1].toInteger() : 0;
    if (scope.engine->hasException)
        RETURN_UNDEFINED();

    if (doffset < 0 || doffset >= UINT_MAX)
        RETURN_RESULT(scope.engine->throwRangeError(QStringLiteral("TypedArray.set: out of range")));
    uint offset = (uint)doffset;
    uint elementSize = a->d()->type->bytesPerElement;

    Scoped<TypedArray> srcTypedArray(scope, argv[0]);
    if (!srcTypedArray) {
        // src is a regular object
        ScopedObject o(scope, argv[0].toObject(scope.engine));
        if (scope.engine->hasException || !o)
            return scope.engine->throwTypeError();

        double len = ScopedValue(scope, o->get(scope.engine->id_length()))->toNumber();
        uint l = (uint)len;
        if (scope.engine->hasException || l != len)
            return scope.engine->throwTypeError();

        if (offset + l > a->length())
            RETURN_RESULT(scope.engine->throwRangeError(QStringLiteral("TypedArray.set: out of range")));

        uint idx = 0;
        char *b = buffer->d()->data->data() + a->d()->byteOffset + offset*elementSize;
        ScopedValue val(scope);
        while (idx < l) {
            val = o->getIndexed(idx);
            a->d()->type->write(scope.engine, b, 0, val);
            if (scope.engine->hasException)
                RETURN_UNDEFINED();
            ++idx;
            b += elementSize;
        }
        RETURN_UNDEFINED();
    }

    // src is a typed array
    Scoped<ArrayBuffer> srcBuffer(scope, srcTypedArray->d()->buffer);
    if (!srcBuffer)
        return scope.engine->throwTypeError();

    uint l = srcTypedArray->length();
    if (offset + l > a->length())
        RETURN_RESULT(scope.engine->throwRangeError(QStringLiteral("TypedArray.set: out of range")));

    char *dest = buffer->d()->data->data() + a->d()->byteOffset + offset*elementSize;
    const char *src = srcBuffer->d()->data->data() + srcTypedArray->d()->byteOffset;
    if (srcTypedArray->d()->type == a->d()->type) {
        // same type of typed arrays, use memmove (as srcbuffer and buffer could be the same)
        memmove(dest, src, srcTypedArray->d()->byteLength);
        RETURN_UNDEFINED();
    }

    char *srcCopy = nullptr;
    if (buffer->d() == srcBuffer->d()) {
        // same buffer, need to take a temporary copy, to not run into problems
        srcCopy = new char[srcTypedArray->d()->byteLength];
        memcpy(srcCopy, src, srcTypedArray->d()->byteLength);
        src = srcCopy;
    }

    // typed arrays of different kind, need to manually loop
    uint srcElementSize = srcTypedArray->d()->type->bytesPerElement;
    TypedArrayRead read = srcTypedArray->d()->type->read;
    TypedArrayWrite write = a->d()->type->write;
    for (uint i = 0; i < l; ++i) {
        Primitive val;
        val.setRawValue(read(src, i*srcElementSize));
        write(scope.engine, dest, i*elementSize, val);
    }

    if (srcCopy)
        delete [] srcCopy;

    RETURN_UNDEFINED();
}

ReturnedValue TypedArrayPrototype::method_subarray(const FunctionObject *builtin, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(builtin);
    Scoped<TypedArray> a(scope, *thisObject);

    if (!a)
        return scope.engine->throwTypeError();

    Scoped<ArrayBuffer> buffer(scope, a->d()->buffer);
    if (!buffer)
        return scope.engine->throwTypeError();

    int len = a->length();
    double b = argc > 0 ? argv[0].toInteger() : 0;
    if (b < 0)
        b = len + b;
    uint begin = (uint)qBound(0., b, (double)len);

    double e = argc < 2 || argv[1].isUndefined() ? len : argv[1].toInteger();
    if (e < 0)
        e = len + e;
    uint end = (uint)qBound(0., e, (double)len);
    if (end < begin)
        end = begin;

    if (scope.engine->hasException)
        RETURN_UNDEFINED();

    int newLen = end - begin;

    ScopedFunctionObject constructor(scope, a->get(scope.engine->id_constructor()));
    if (!constructor)
        return scope.engine->throwTypeError();

    Value *arguments = scope.alloc(3);
    arguments[0] = buffer;
    arguments[1] = Encode(a->d()->byteOffset + begin*a->d()->type->bytesPerElement);
    arguments[2] = Encode(newLen);
    return constructor->callAsConstructor(arguments, 3);
}
