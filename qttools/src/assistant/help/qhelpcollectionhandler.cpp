/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Assistant of the Qt Toolkit.
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

#include "qhelpcollectionhandler_p.h"
#include "qhelp_global.h"
#include "qhelpdbreader_p.h"

#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QDebug>

#include <QtSql/QSqlError>
#include <QtSql/QSqlDriver>

QT_BEGIN_NAMESPACE

QHelpCollectionHandler::QHelpCollectionHandler(const QString &collectionFile, QObject *parent)
    : QObject(parent)
    , m_collectionFile(collectionFile)
{
    const QFileInfo fi(m_collectionFile);
    if (!fi.isAbsolute())
        m_collectionFile = fi.absoluteFilePath();
}

QHelpCollectionHandler::~QHelpCollectionHandler()
{
    closeDB();
}

bool QHelpCollectionHandler::isDBOpened()
{
    if (m_query)
        return true;
    emit error(tr("The collection file \"%1\" is not set up yet.").
               arg(m_collectionFile));
    return false;
}

void QHelpCollectionHandler::closeDB()
{
    if (!m_query)
        return;

    delete m_query;
    m_query = nullptr;
    QSqlDatabase::removeDatabase(m_connectionName);
    m_connectionName = QString();
}

QString QHelpCollectionHandler::collectionFile() const
{
    return m_collectionFile;
}

bool QHelpCollectionHandler::openCollectionFile()
{
    if (m_query)
        return true;

    m_connectionName = QHelpGlobal::uniquifyConnectionName(
        QLatin1String("QHelpCollectionHandler"), this);
    {
        QSqlDatabase db = QSqlDatabase::addDatabase(QLatin1String("QSQLITE"),
            m_connectionName);
        if (db.driver()
            && db.driver()->lastError().type() == QSqlError::ConnectionError) {
            emit error(tr("Cannot load sqlite database driver."));
            return false;
        }

        db.setDatabaseName(collectionFile());
        if (db.open())
            m_query = new QSqlQuery(db);

        if (!m_query) {
            QSqlDatabase::removeDatabase(m_connectionName);
            emit error(tr("Cannot open collection file: %1").arg(collectionFile()));
            return false;
        }
    }

    m_query->exec(QLatin1String("PRAGMA synchronous=OFF"));
    m_query->exec(QLatin1String("PRAGMA cache_size=3000"));

    m_query->exec(QLatin1String("SELECT COUNT(*) FROM sqlite_master WHERE TYPE=\'table\' "
                                "AND Name=\'NamespaceTable\'"));
    m_query->next();
    if (m_query->value(0).toInt() < 1) {
        if (!createTables(m_query)) {
            closeDB();
            emit error(tr("Cannot create tables in file %1.").arg(collectionFile()));
            return false;
        }
    }

    return true;
}

bool QHelpCollectionHandler::copyCollectionFile(const QString &fileName)
{
    if (!m_query)
        return false;

    const QFileInfo fi(fileName);
    if (fi.exists()) {
        emit error(tr("The collection file \"%1\" already exists.").
                   arg(fileName));
        return false;
    }

    if (!fi.absoluteDir().exists() && !QDir().mkpath(fi.absolutePath())) {
        emit error(tr("Cannot create directory: %1").arg(fi.absolutePath()));
        return false;
    }

    const QString &colFile = fi.absoluteFilePath();
    const QString &connectionName = QHelpGlobal::uniquifyConnectionName(
                QLatin1String("QHelpCollectionHandlerCopy"), this);
    QSqlQuery *copyQuery = 0;
    bool openingOk = true;
    {
        QSqlDatabase db = QSqlDatabase::addDatabase(QLatin1String("QSQLITE"), connectionName);
        db.setDatabaseName(colFile);
        openingOk = db.open();
        if (openingOk)
            copyQuery = new QSqlQuery(db);
    }

    if (!openingOk) {
        emit error(tr("Cannot open collection file: %1").arg(colFile));
        return false;
    }

    copyQuery->exec(QLatin1String("PRAGMA synchronous=OFF"));
    copyQuery->exec(QLatin1String("PRAGMA cache_size=3000"));

    if (!createTables(copyQuery)) {
        emit error(tr("Cannot copy collection file: %1").arg(colFile));
        return false;
    }

    const QString &oldBaseDir = QFileInfo(collectionFile()).absolutePath();
    const QFileInfo newColFi(colFile);
    m_query->exec(QLatin1String("SELECT Name, FilePath FROM NamespaceTable"));
    while (m_query->next()) {
        copyQuery->prepare(QLatin1String("INSERT INTO NamespaceTable VALUES(NULL, ?, ?)"));
        copyQuery->bindValue(0, m_query->value(0).toString());
        QString oldFilePath = m_query->value(1).toString();
        if (!QDir::isAbsolutePath(oldFilePath))
            oldFilePath = oldBaseDir + QDir::separator() + oldFilePath;
        copyQuery->bindValue(1, newColFi.absoluteDir().relativeFilePath(oldFilePath));
        copyQuery->exec();
    }

    m_query->exec(QLatin1String("SELECT NamespaceId, Name FROM FolderTable"));
    while (m_query->next()) {
        copyQuery->prepare(QLatin1String("INSERT INTO FolderTable VALUES(NULL, ?, ?)"));
        copyQuery->bindValue(0, m_query->value(0).toString());
        copyQuery->bindValue(1, m_query->value(1).toString());
        copyQuery->exec();
    }

    m_query->exec(QLatin1String("SELECT Name FROM FilterAttributeTable"));
    while (m_query->next()) {
        copyQuery->prepare(QLatin1String("INSERT INTO FilterAttributeTable VALUES(NULL, ?)"));
        copyQuery->bindValue(0, m_query->value(0).toString());
        copyQuery->exec();
    }

    m_query->exec(QLatin1String("SELECT Name FROM FilterNameTable"));
    while (m_query->next()) {
        copyQuery->prepare(QLatin1String("INSERT INTO FilterNameTable VALUES(NULL, ?)"));
        copyQuery->bindValue(0, m_query->value(0).toString());
        copyQuery->exec();
    }

    m_query->exec(QLatin1String("SELECT NameId, FilterAttributeId FROM FilterTable"));
    while (m_query->next()) {
        copyQuery->prepare(QLatin1String("INSERT INTO FilterTable VALUES(?, ?)"));
        copyQuery->bindValue(0, m_query->value(0).toInt());
        copyQuery->bindValue(1, m_query->value(1).toInt());
        copyQuery->exec();
    }

    m_query->exec(QLatin1String("SELECT Key, Value FROM SettingsTable"));
    while (m_query->next()) {
        if (m_query->value(0).toString() == QLatin1String("FTS5IndexedNamespaces"))
            continue;
        copyQuery->prepare(QLatin1String("INSERT INTO SettingsTable VALUES(?, ?)"));
        copyQuery->bindValue(0, m_query->value(0).toString());
        copyQuery->bindValue(1, m_query->value(1));
        copyQuery->exec();
    }

    copyQuery->clear();
    delete copyQuery;
    QSqlDatabase::removeDatabase(connectionName);
    return true;
}

bool QHelpCollectionHandler::createTables(QSqlQuery *query)
{
    const QStringList tables = QStringList()
            << QLatin1String("CREATE TABLE NamespaceTable ("
                             "Id INTEGER PRIMARY KEY, "
                             "Name TEXT, "
                             "FilePath TEXT )")
            << QLatin1String("CREATE TABLE FolderTable ("
                             "Id INTEGER PRIMARY KEY, "
                             "NamespaceId INTEGER, "
                             "Name TEXT )")
            << QLatin1String("CREATE TABLE FilterAttributeTable ("
                             "Id INTEGER PRIMARY KEY, "
                             "Name TEXT )")
            << QLatin1String("CREATE TABLE FilterNameTable ("
                             "Id INTEGER PRIMARY KEY, "
                             "Name TEXT )")
            << QLatin1String("CREATE TABLE FilterTable ("
                             "NameId INTEGER, "
                             "FilterAttributeId INTEGER )")
            << QLatin1String("CREATE TABLE SettingsTable ("
                             "Key TEXT PRIMARY KEY, "
                             "Value BLOB )");

    for (const QString &q : tables) {
        if (!query->exec(q))
            return false;
    }
    return true;
}

QStringList QHelpCollectionHandler::customFilters() const
{
    QStringList list;
    if (m_query) {
        m_query->exec(QLatin1String("SELECT Name FROM FilterNameTable"));
        while (m_query->next())
            list.append(m_query->value(0).toString());
    }
    return list;
}

bool QHelpCollectionHandler::removeCustomFilter(const QString &filterName)
{
    if (!isDBOpened() || filterName.isEmpty())
        return false;

    int filterNameId = -1;
    m_query->prepare(QLatin1String("SELECT Id FROM FilterNameTable WHERE Name=?"));
    m_query->bindValue(0, filterName);
    m_query->exec();
    if (m_query->next())
        filterNameId = m_query->value(0).toInt();

    if (filterNameId < 0) {
        emit error(tr("Unknown filter \"%1\".").arg(filterName));
        return false;
    }

    m_query->prepare(QLatin1String("DELETE FROM FilterTable WHERE NameId=?"));
    m_query->bindValue(0, filterNameId);
    m_query->exec();

    m_query->prepare(QLatin1String("DELETE FROM FilterNameTable WHERE Id=?"));
    m_query->bindValue(0, filterNameId);
    m_query->exec();

    return true;
}

bool QHelpCollectionHandler::addCustomFilter(const QString &filterName,
                                             const QStringList &attributes)
{
    if (!isDBOpened() || filterName.isEmpty())
        return false;

    int nameId = -1;
    m_query->prepare(QLatin1String("SELECT Id FROM FilterNameTable WHERE Name=?"));
    m_query->bindValue(0, filterName);
    m_query->exec();
    if (m_query->next())
        nameId = m_query->value(0).toInt();

    m_query->exec(QLatin1String("SELECT Id, Name FROM FilterAttributeTable"));
    QStringList idsToInsert = attributes;
    QMap<QString, int> attributeMap;
    while (m_query->next()) {
        attributeMap.insert(m_query->value(1).toString(),
            m_query->value(0).toInt());
        if (idsToInsert.contains(m_query->value(1).toString()))
            idsToInsert.removeAll(m_query->value(1).toString());
    }

    for (const QString &id : qAsConst(idsToInsert)) {
        m_query->prepare(QLatin1String("INSERT INTO FilterAttributeTable VALUES(NULL, ?)"));
        m_query->bindValue(0, id);
        m_query->exec();
        attributeMap.insert(id, m_query->lastInsertId().toInt());
    }

    if (nameId < 0) {
        m_query->prepare(QLatin1String("INSERT INTO FilterNameTable VALUES(NULL, ?)"));
        m_query->bindValue(0, filterName);
        if (m_query->exec())
            nameId = m_query->lastInsertId().toInt();
    }

    if (nameId < 0) {
        emit error(tr("Cannot register filter %1.").arg(filterName));
        return false;
    }

    m_query->prepare(QLatin1String("DELETE FROM FilterTable WHERE NameId=?"));
    m_query->bindValue(0, nameId);
    m_query->exec();

    for (const QString &att : attributes) {
        m_query->prepare(QLatin1String("INSERT INTO FilterTable VALUES(?, ?)"));
        m_query->bindValue(0, nameId);
        m_query->bindValue(1, attributeMap[att]);
        if (!m_query->exec())
            return false;
    }
    return true;
}

QHelpCollectionHandler::DocInfoList QHelpCollectionHandler::registeredDocumentations(
        const QString &namespaceName) const
{
    DocInfoList list;

    if (!m_query)
        return list;

    static const QLatin1String baseQuery("SELECT a.Name, a.FilePath, b.Name "
                                         "FROM NamespaceTable a, FolderTable b "
                                         "WHERE a.Id=b.NamespaceId");

    if (namespaceName.isEmpty()) {
        m_query->prepare(baseQuery);
    } else {
        m_query->prepare(baseQuery + QLatin1String(" AND a.Name=? LIMIT 1"));
        m_query->bindValue(0, namespaceName);
    }

    m_query->exec();

    while (m_query->next()) {
        DocInfo info;
        info.namespaceName = m_query->value(0).toString();
        info.fileName = m_query->value(1).toString();
        info.folderName = m_query->value(2).toString();
        list.append(info);
    }

    return list;
}

bool QHelpCollectionHandler::registerDocumentation(const QString &fileName)
{
    if (!isDBOpened())
        return false;

    QHelpDBReader reader(fileName, QHelpGlobal::uniquifyConnectionName(
        QLatin1String("QHelpCollectionHandler"), this), 0);
    if (!reader.init()) {
        emit error(tr("Cannot open documentation file %1.").arg(fileName));
        return false;
    }

    const QString &ns = reader.namespaceName();
    if (ns.isEmpty()) {
        emit error(tr("Invalid documentation file \"%1\".").arg(fileName));
        return false;
    }

    const int nsId = registerNamespace(ns, fileName);
    if (nsId < 1)
        return false;

    if (!registerVirtualFolder(reader.virtualFolder(), nsId))
        return false;

    addFilterAttributes(reader.filterAttributes());
    for (const QString &filterName : reader.customFilters())
        addCustomFilter(filterName, reader.filterAttributes(filterName));

    optimizeDatabase(fileName);

    return true;
}

bool QHelpCollectionHandler::unregisterDocumentation(const QString &namespaceName)
{
    if (!isDBOpened())
        return false;

    m_query->prepare(QLatin1String("SELECT Id FROM NamespaceTable WHERE Name = ?"));
    m_query->bindValue(0, namespaceName);
    m_query->exec();

    if (!m_query->next()) {
        emit error(tr("The namespace %1 was not registered.").arg(namespaceName));
        return false;
    }

    const int nsId = m_query->value(0).toInt();

    m_query->prepare(QLatin1String("DELETE FROM NamespaceTable WHERE Id = ?"));
    m_query->bindValue(0, nsId);
    if (!m_query->exec())
        return false;

    m_query->prepare(QLatin1String("DELETE FROM FolderTable WHERE NamespaceId = ?"));
    m_query->bindValue(0, nsId);
    if (!m_query->exec())
        return false;

    return true;
}

bool QHelpCollectionHandler::removeCustomValue(const QString &key)
{
    if (!isDBOpened())
        return false;

    m_query->prepare(QLatin1String("DELETE FROM SettingsTable WHERE Key=?"));
    m_query->bindValue(0, key);
    return m_query->exec();
}

QVariant QHelpCollectionHandler::customValue(const QString &key,
                                             const QVariant &defaultValue) const
{
    if (!m_query)
        return defaultValue;

    m_query->prepare(QLatin1String("SELECT COUNT(Key) FROM SettingsTable WHERE Key=?"));
    m_query->bindValue(0, key);
    if (!m_query->exec() || !m_query->next() || !m_query->value(0).toInt()) {
        m_query->clear();
        return defaultValue;
    }

    m_query->clear();
    m_query->prepare(QLatin1String("SELECT Value FROM SettingsTable WHERE Key=?"));
    m_query->bindValue(0, key);
    if (m_query->exec() && m_query->next()) {
        const QVariant &value = m_query->value(0);
        m_query->clear();
        return value;
    }

    return defaultValue;
}

bool QHelpCollectionHandler::setCustomValue(const QString &key,
                                            const QVariant &value)
{
    if (!isDBOpened())
        return false;

    m_query->prepare(QLatin1String("SELECT Value FROM SettingsTable WHERE Key=?"));
    m_query->bindValue(0, key);
    m_query->exec();
    if (m_query->next()) {
        m_query->prepare(QLatin1String("UPDATE SettingsTable SET Value=? where Key=?"));
        m_query->bindValue(0, value);
        m_query->bindValue(1, key);
    } else {
        m_query->prepare(QLatin1String("INSERT INTO SettingsTable VALUES(?, ?)"));
        m_query->bindValue(0, key);
        m_query->bindValue(1, value);
    }
    return m_query->exec();
}

bool QHelpCollectionHandler::addFilterAttributes(const QStringList &attributes)
{
    if (!isDBOpened())
        return false;

    m_query->exec(QLatin1String("SELECT Name FROM FilterAttributeTable"));
    QSet<QString> atts;
    while (m_query->next())
        atts.insert(m_query->value(0).toString());

    for (const QString &s : attributes) {
        if (!atts.contains(s)) {
            m_query->prepare(QLatin1String("INSERT INTO FilterAttributeTable VALUES(NULL, ?)"));
            m_query->bindValue(0, s);
            m_query->exec();
        }
    }
    return true;
}

QStringList QHelpCollectionHandler::filterAttributes() const
{
    QStringList list;
    if (m_query) {
        m_query->exec(QLatin1String("SELECT Name FROM FilterAttributeTable"));
        while (m_query->next())
            list.append(m_query->value(0).toString());
    }
    return list;
}

QStringList QHelpCollectionHandler::filterAttributes(const QString &filterName) const
{
    QStringList list;
    if (m_query) {
        m_query->prepare(QLatin1String("SELECT a.Name FROM FilterAttributeTable a, "
            "FilterTable b, FilterNameTable c WHERE a.Id=b.FilterAttributeId "
            "AND b.NameId=c.Id AND c.Name=?"));
        m_query->bindValue(0, filterName);
        m_query->exec();
        while (m_query->next())
            list.append(m_query->value(0).toString());
    }
    return list;
}

int QHelpCollectionHandler::registerNamespace(const QString &nspace, const QString &fileName)
{
    const int errorValue = -1;
    if (!m_query)
        return errorValue;

    m_query->prepare(QLatin1String("SELECT COUNT(Id) FROM NamespaceTable WHERE Name=?"));
    m_query->bindValue(0, nspace);
    m_query->exec();
    while (m_query->next()) {
        if (m_query->value(0).toInt() > 0) {
            emit error(tr("Namespace %1 already exists.").arg(nspace));
            return errorValue;
        }
    }

    QFileInfo fi(m_collectionFile);
    m_query->prepare(QLatin1String("INSERT INTO NamespaceTable VALUES(NULL, ?, ?)"));
    m_query->bindValue(0, nspace);
    m_query->bindValue(1, fi.absoluteDir().relativeFilePath(fileName));
    int namespaceId = errorValue;
    if (m_query->exec())
        namespaceId = m_query->lastInsertId().toInt();
    if (namespaceId < 1) {
        emit error(tr("Cannot register namespace \"%1\".").arg(nspace));
        return errorValue;
    }
    return namespaceId;
}

bool QHelpCollectionHandler::registerVirtualFolder(const QString &folderName, int namespaceId)
{
    if (!m_query)
        return false;

    m_query->prepare(QLatin1String("INSERT INTO FolderTable VALUES(NULL, ?, ?)"));
    m_query->bindValue(0, namespaceId);
    m_query->bindValue(1, folderName);
    return m_query->exec();
}

void QHelpCollectionHandler::optimizeDatabase(const QString &fileName)
{
    if (!QFile::exists(fileName))
        return;

    {   // according to removeDatabase() documentation
        QSqlDatabase db = QSqlDatabase::addDatabase(QLatin1String("QSQLITE"), QLatin1String("optimize"));
        db.setDatabaseName(fileName);
        if (!db.open()) {
            QSqlDatabase::removeDatabase(QLatin1String("optimize"));
            emit error(tr("Cannot open database \"%1\" to optimize.").arg(fileName));
            return;
        }

        db.exec(QLatin1String("PRAGMA synchronous=OFF"));
        db.exec(QLatin1String("PRAGMA cache_size=3000"));
        db.exec(QLatin1String("CREATE INDEX IF NOT EXISTS NameIndex ON IndexTable(Name)"));
        db.exec(QLatin1String("CREATE INDEX IF NOT EXISTS FileNameIndex ON FileNameTable(Name)"));
        db.exec(QLatin1String("CREATE INDEX IF NOT EXISTS FileIdIndex ON FileNameTable(FileId)"));

        db.close();
    }

    QSqlDatabase::removeDatabase(QLatin1String("optimize"));
}

QT_END_NAMESPACE
