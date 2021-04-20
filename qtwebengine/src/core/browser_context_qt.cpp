/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
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

#include "browser_context_qt.h"

#include "browser_context_adapter.h"
#include "browsing_data_remover_delegate_qt.h"
#include "download_manager_delegate_qt.h"
#include "net/ssl_host_state_delegate_qt.h"
#include "net/url_request_context_getter_qt.h"
#include "permission_manager_qt.h"
#include "qtwebenginecoreglobal_p.h"
#include "resource_context_qt.h"
#include "type_conversion.h"
#include "web_engine_library_info.h"

#include "base/time/time.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/storage_partition.h"
#include "net/proxy/proxy_config_service.h"

#include "base/base_paths.h"
#include "components/prefs/pref_member.h"
#include "components/prefs/pref_service.h"
#include "components/prefs/in_memory_pref_store.h"
#include "components/prefs/pref_service.h"
#include "components/prefs/pref_service_factory.h"
#include "components/prefs/pref_registry_simple.h"
#include "components/user_prefs/user_prefs.h"
#if BUILDFLAG(ENABLE_SPELLCHECK)
#include "chrome/browser/spellchecker/spellcheck_service.h"
#include "chrome/common/pref_names.h"
#include "components/spellcheck/browser/pref_names.h"
#endif

namespace QtWebEngineCore {

BrowserContextQt::BrowserContextQt(BrowserContextAdapter *adapter)
    : m_adapter(adapter)
{
    PrefServiceFactory factory;
    factory.set_user_prefs(new InMemoryPrefStore);
    PrefRegistrySimple *registry = new PrefRegistrySimple();

#if BUILDFLAG(ENABLE_SPELLCHECK)
    // Initial spellcheck settings
    registry->RegisterStringPref(prefs::kAcceptLanguages, std::string());
    registry->RegisterListPref(spellcheck::prefs::kSpellCheckDictionaries, base::MakeUnique<base::ListValue>());
    registry->RegisterListPref(spellcheck::prefs::kSpellCheckForcedDictionaries, base::MakeUnique<base::ListValue>());
    registry->RegisterStringPref(spellcheck::prefs::kSpellCheckDictionary, std::string());
    registry->RegisterBooleanPref(spellcheck::prefs::kSpellCheckEnable, false);
    registry->RegisterBooleanPref(spellcheck::prefs::kSpellCheckUseSpellingService, false);
#endif //ENABLE_SPELLCHECK
    m_prefService = factory.Create(registry);
    user_prefs::UserPrefs::Set(this, m_prefService.get());
}

BrowserContextQt::~BrowserContextQt()
{
    if (resourceContext)
        content::BrowserThread::DeleteSoon(content::BrowserThread::IO, FROM_HERE, resourceContext.release());
}

PrefService* BrowserContextQt::GetPrefs()
{
    return m_prefService.get();
}

const PrefService* BrowserContextQt::GetPrefs() const
{
    return m_prefService.get();
}

base::FilePath BrowserContextQt::GetPath() const
{
    return toFilePath(m_adapter->dataPath());
}

bool BrowserContextQt::IsOffTheRecord() const
{
    return m_adapter->isOffTheRecord();
}

net::URLRequestContextGetter *BrowserContextQt::GetRequestContext()
{
    return url_request_getter_.get();
}

net::URLRequestContextGetter *BrowserContextQt::CreateMediaRequestContext()
{
    return url_request_getter_.get();
}

net::URLRequestContextGetter *BrowserContextQt::CreateMediaRequestContextForStoragePartition(const base::FilePath&, bool)
{
    Q_UNIMPLEMENTED();
    return nullptr;
}

content::ResourceContext *BrowserContextQt::GetResourceContext()
{
    if (!resourceContext)
        resourceContext.reset(new ResourceContextQt(this));
    return resourceContext.get();
}

content::DownloadManagerDelegate *BrowserContextQt::GetDownloadManagerDelegate()
{
    return m_adapter->downloadManagerDelegate();
}

content::BrowserPluginGuestManager *BrowserContextQt::GetGuestManager()
{
    return 0;
}

storage::SpecialStoragePolicy *BrowserContextQt::GetSpecialStoragePolicy()
{
    QT_NOT_YET_IMPLEMENTED
    return 0;
}

content::PushMessagingService *BrowserContextQt::GetPushMessagingService()
{
    return 0;
}

content::SSLHostStateDelegate* BrowserContextQt::GetSSLHostStateDelegate()
{
    if (!sslHostStateDelegate)
        sslHostStateDelegate.reset(new SSLHostStateDelegateQt());
    return sslHostStateDelegate.get();
}

std::unique_ptr<content::ZoomLevelDelegate> BrowserContextQt::CreateZoomLevelDelegate(const base::FilePath&)
{
    return nullptr;
}

content::BackgroundFetchDelegate* BrowserContextQt::GetBackgroundFetchDelegate()
{
    return nullptr;
}

content::BackgroundSyncController* BrowserContextQt::GetBackgroundSyncController()
{
    return nullptr;
}

content::BrowsingDataRemoverDelegate *BrowserContextQt::GetBrowsingDataRemoverDelegate()
{
    return new BrowsingDataRemoverDelegateQt;
}

content::PermissionManager *BrowserContextQt::GetPermissionManager()
{
    if (!permissionManager)
        permissionManager.reset(new PermissionManagerQt());
    return permissionManager.get();
}

net::URLRequestContextGetter *BrowserContextQt::CreateRequestContext(content::ProtocolHandlerMap *protocol_handlers, content::URLRequestInterceptorScopedVector request_interceptors)
{
    url_request_getter_ = new URLRequestContextGetterQt(m_adapter->sharedFromThis(), protocol_handlers, std::move(request_interceptors));
    return url_request_getter_.get();
}

net::URLRequestContextGetter *BrowserContextQt::CreateRequestContextForStoragePartition(
        const base::FilePath& partition_path, bool in_memory,
        content::ProtocolHandlerMap* protocol_handlers,
        content::URLRequestInterceptorScopedVector request_interceptors)
{
    Q_UNIMPLEMENTED();
    return nullptr;
}

#if BUILDFLAG(ENABLE_SPELLCHECK)
void BrowserContextQt::FailedToLoadDictionary(const std::string &language)
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
    qWarning() << "Could not load dictionary for:" << toQt(language) << endl
               << "Make sure that correct bdic file is in:" << toQt(WebEngineLibraryInfo::getPath(base::DIR_APP_DICTIONARIES).value());
}

void BrowserContextQt::setSpellCheckLanguages(const QStringList &languages)
{
    StringListPrefMember dictionaries_pref;
    dictionaries_pref.Init(spellcheck::prefs::kSpellCheckDictionaries, m_prefService.get());
    std::vector<std::string> dictionaries;
    dictionaries.reserve(languages.size());
    for (const auto &language : languages)
        dictionaries.push_back(language.toStdString());
    dictionaries_pref.SetValue(dictionaries);
}

QStringList BrowserContextQt::spellCheckLanguages() const
{
    QStringList spellcheck_dictionaries;
    for (const auto &value : *m_prefService->GetList(spellcheck::prefs::kSpellCheckDictionaries)) {
        std::string dictionary;
        if (value.GetAsString(&dictionary))
            spellcheck_dictionaries.append(QString::fromStdString(dictionary));
    }

    return spellcheck_dictionaries;
}

void BrowserContextQt::setSpellCheckEnabled(bool enabled)
{
    m_prefService->SetBoolean(spellcheck::prefs::kSpellCheckEnable, enabled);
}

bool BrowserContextQt::isSpellCheckEnabled() const
{
    return m_prefService->GetBoolean(spellcheck::prefs::kSpellCheckEnable);
}
#endif //ENABLE_SPELLCHECK
} // namespace QtWebEngineCore
