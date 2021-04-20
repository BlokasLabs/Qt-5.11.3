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

// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.Chromium file.

#include "print_view_manager_qt.h"

#include "type_conversion.h"
#include "web_engine_context.h"

#include <QtGui/qpagelayout.h>
#include <QtGui/qpagesize.h>

#include "base/values.h"
#include "base/memory/ref_counted_memory.h"
#include "chrome/browser/printing/print_job_manager.h"
#include "chrome/browser/printing/printer_query.h"
#include "components/printing/common/print_messages.h"
#include "content/browser/renderer_host/render_view_host_impl.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/common/web_preferences.h"
#include "printing/pdf_metafile_skia.h"
#include "printing/print_job_constants.h"
#include "printing/units.h"

DEFINE_WEB_CONTENTS_USER_DATA_KEY(QtWebEngineCore::PrintViewManagerQt);

namespace {
static const qreal kMicronsToMillimeter = 1000.0f;

static std::vector<char>
GetStdVectorFromHandle(base::SharedMemoryHandle handle, uint32_t data_size)
{
    std::unique_ptr<base::SharedMemory> shared_buf(
                new base::SharedMemory(handle, true));

    if (!shared_buf->Map(data_size)) {
        return std::vector<char>();
    }

    char* data = static_cast<char*>(shared_buf->memory());
    return std::vector<char>(data, data + data_size);
}

static scoped_refptr<base::RefCountedBytes>
GetBytesFromHandle(base::SharedMemoryHandle handle, uint32_t data_size)
{
     std::unique_ptr<base::SharedMemory> shared_buf(
                 new base::SharedMemory(handle, true));

    if (!shared_buf->Map(data_size)) {
       return NULL;
    }

    unsigned char* data = static_cast<unsigned char*>(shared_buf->memory());
    std::vector<unsigned char> dataVector(data, data + data_size);
    return base::RefCountedBytes::TakeVector(&dataVector);
}

// Write the PDF file to disk.
static void SavePdfFile(scoped_refptr<base::RefCountedBytes> data,
                        const base::FilePath& path,
                        const QtWebEngineCore::PrintViewManagerQt::PrintToPDFFileCallback
                                &saveCallback)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::FILE);
    DCHECK_GT(data->size(), 0U);

    printing::PdfMetafileSkia metafile(printing::SkiaDocumentType::PDF);
    metafile.InitFromData(static_cast<const void*>(data->front()), data->size());

    base::File file(path,
                    base::File::FLAG_CREATE_ALWAYS | base::File::FLAG_WRITE);
    bool success = file.IsValid() && metafile.SaveTo(&file);
    content::BrowserThread::PostTask(content::BrowserThread::UI,
                                     FROM_HERE,
                                     base::Bind(saveCallback, success));
}

static base::DictionaryValue *createPrintSettings()
{
    base::DictionaryValue *printSettings = new base::DictionaryValue();
    // TO DO: Check if we can use the request ID from Qt here somehow.
    static int internalRequestId = 0;

    printSettings->SetBoolean(printing::kIsFirstRequest, internalRequestId++ == 0);
    printSettings->SetInteger(printing::kPreviewRequestID, internalRequestId);

    // The following are standard settings that Chromium expects to be set.
    printSettings->SetBoolean(printing::kSettingPrintToPDF, true);
    printSettings->SetBoolean(printing::kSettingCloudPrintDialog, false);
    printSettings->SetBoolean(printing::kSettingPrintWithPrivet, false);
    printSettings->SetBoolean(printing::kSettingPrintWithExtension, false);

    printSettings->SetBoolean(printing::kSettingGenerateDraftData, false);
    printSettings->SetBoolean(printing::kSettingPreviewModifiable, false);

    printSettings->SetInteger(printing::kSettingDpiHorizontal, printing::kPointsPerInch);
    printSettings->SetInteger(printing::kSettingDpiVertical, printing::kPointsPerInch);

    printSettings->SetInteger(printing::kSettingDuplexMode, printing::SIMPLEX);
    printSettings->SetInteger(printing::kSettingCopies, 1);
    printSettings->SetBoolean(printing::kSettingCollate, false);
    printSettings->SetBoolean(printing::kSettingGenerateDraftData, false);
    printSettings->SetBoolean(printing::kSettingPreviewModifiable, false);

    printSettings->SetBoolean(printing::kSettingShouldPrintSelectionOnly, false);
    printSettings->SetBoolean(printing::kSettingShouldPrintBackgrounds, true);
    printSettings->SetBoolean(printing::kSettingHeaderFooterEnabled, false);
    printSettings->SetBoolean(printing::kSettingRasterizePdf, false);
    printSettings->SetInteger(printing::kSettingScaleFactor, 100);
    printSettings->SetString(printing::kSettingDeviceName, "");
    printSettings->SetInteger(printing::kPreviewUIID, 12345678);

    return printSettings;
}

static base::DictionaryValue *createPrintSettingsFromQPageLayout(const QPageLayout &pageLayout,
                                                                 bool useCustomMargins)
{
    base::DictionaryValue *printSettings = createPrintSettings();

    //Set page size attributes, chromium expects these in micrometers
    QRectF pageSizeInMillimeter = pageLayout.pageSize().rect(QPageSize::Millimeter);
    if (!useCustomMargins) {
        // QPrinter will extend this size with its margins
        QMarginsF margins = pageLayout.margins(QPageLayout::Millimeter);
        pageSizeInMillimeter = pageSizeInMillimeter.marginsRemoved(margins);
    }
    std::unique_ptr<base::DictionaryValue> sizeDict(new base::DictionaryValue);
    sizeDict->SetInteger(printing::kSettingMediaSizeWidthMicrons, pageSizeInMillimeter.width() * kMicronsToMillimeter);
    sizeDict->SetInteger(printing::kSettingMediaSizeHeightMicrons, pageSizeInMillimeter.height() * kMicronsToMillimeter);
    printSettings->Set(printing::kSettingMediaSize, std::move(sizeDict));

    if (useCustomMargins) {
        // Apply page margins when printing to PDF
        QMargins pageMarginsInPoints = pageLayout.marginsPoints();
        std::unique_ptr<base::DictionaryValue> marginsDict(new base::DictionaryValue);
        marginsDict->SetInteger(printing::kSettingMarginTop, pageMarginsInPoints.top());
        marginsDict->SetInteger(printing::kSettingMarginBottom, pageMarginsInPoints.bottom());
        marginsDict->SetInteger(printing::kSettingMarginLeft, pageMarginsInPoints.left());
        marginsDict->SetInteger(printing::kSettingMarginRight, pageMarginsInPoints.right());

        printSettings->Set(printing::kSettingMarginsCustom, std::move(marginsDict));
        printSettings->SetInteger(printing::kSettingMarginsType, printing::CUSTOM_MARGINS);
    } else {
        // QPrinter will handle margins
        printSettings->SetInteger(printing::kSettingMarginsType, printing::NO_MARGINS);
    }

    printSettings->SetBoolean(printing::kSettingLandscape, pageLayout.orientation() == QPageLayout::Landscape);

    return printSettings;
}

} // namespace

namespace QtWebEngineCore {

PrintViewManagerQt::~PrintViewManagerQt()
{
}

#if BUILDFLAG(ENABLE_BASIC_PRINTING)
void PrintViewManagerQt::PrintToPDFFileWithCallback(const QPageLayout &pageLayout,
                                                    bool printInColor,
                                                    const QString &filePath,
                                                    const PrintToPDFFileCallback& callback)
{
    if (callback.is_null())
        return;

    if (m_printSettings || !filePath.length()) {
                content::BrowserThread::PostTask(content::BrowserThread::UI,
                                         FROM_HERE,
                                         base::Bind(callback, false));
        return;
    }

    m_pdfOutputPath = toFilePath(filePath);
    m_pdfSaveCallback = callback;
    if (!PrintToPDFInternal(pageLayout, printInColor)) {
        content::BrowserThread::PostTask(content::BrowserThread::UI,
                                         FROM_HERE,
                                         base::Bind(callback, false));
        resetPdfState();
    }
}

void PrintViewManagerQt::PrintToPDFWithCallback(const QPageLayout &pageLayout,
                                                bool printInColor,
                                                bool useCustomMargins,
                                                const PrintToPDFCallback& callback)
{
    if (callback.is_null())
        return;

    // If there already is a pending print in progress, don't try starting another one.
    if (m_printSettings) {
            content::BrowserThread::PostTask(content::BrowserThread::UI,
                                             FROM_HERE,
                                             base::Bind(callback, std::vector<char>()));
        return;
    }

    m_pdfPrintCallback = callback;
    if (!PrintToPDFInternal(pageLayout, printInColor, useCustomMargins)) {
        content::BrowserThread::PostTask(content::BrowserThread::UI,
                                         FROM_HERE,
                                         base::Bind(callback, std::vector<char>()));

        resetPdfState();
    }
}

bool PrintViewManagerQt::PrintToPDFInternal(const QPageLayout &pageLayout,
                                            const bool printInColor,
                                            const bool useCustomMargins)
{
    if (!pageLayout.isValid())
        return false;

    m_printSettings.reset(createPrintSettingsFromQPageLayout(pageLayout, useCustomMargins));
    m_printSettings->SetBoolean(printing::kSettingShouldPrintBackgrounds
        , web_contents()->GetRenderViewHost()->GetWebkitPreferences().should_print_backgrounds);
    m_printSettings->SetInteger(printing::kSettingColor,
                                printInColor ? printing::COLOR : printing::GRAYSCALE);
    return web_contents()->GetMainFrame()->Send(
                new PrintMsg_InitiatePrintPreview(web_contents()->GetMainFrame()->GetRoutingID(), false));
}

#endif // BUILDFLAG(ENABLE_BASIC_PRINTING)

// PrintedPagesSource implementation.
base::string16 PrintViewManagerQt::RenderSourceName()
{
     return toString16(QLatin1String(""));
}

PrintViewManagerQt::PrintViewManagerQt(content::WebContents *contents)
    : PrintViewManagerBaseQt(contents)
{

}

// content::WebContentsObserver implementation.
bool PrintViewManagerQt::OnMessageReceived(const IPC::Message& message, content::RenderFrameHost* render_frame_host)
{
    bool handled = true;
    IPC_BEGIN_MESSAGE_MAP(PrintViewManagerQt, message)
      IPC_MESSAGE_HANDLER(PrintHostMsg_DidShowPrintDialog, OnDidShowPrintDialog)
      IPC_MESSAGE_HANDLER(PrintHostMsg_RequestPrintPreview, OnRequestPrintPreview)
      IPC_MESSAGE_HANDLER(PrintHostMsg_MetafileReadyForPrinting, OnMetafileReadyForPrinting);
      IPC_MESSAGE_UNHANDLED(handled = false)
    IPC_END_MESSAGE_MAP()
    return handled || PrintManager::OnMessageReceived(message, render_frame_host);
}

void PrintViewManagerQt::resetPdfState()
{
    m_pdfOutputPath.clear();
    m_pdfPrintCallback.Reset();
    m_pdfSaveCallback.Reset();
    m_printSettings.reset();
}

// IPC handlers

void PrintViewManagerQt::OnRequestPrintPreview(
    const PrintHostMsg_RequestPrintPreview_Params &/*params*/)
{
    auto *rfh = web_contents()->GetMainFrame();
    rfh->Send(new PrintMsg_PrintPreview(rfh->GetRoutingID(), *m_printSettings));
    rfh->Send(new PrintMsg_ClosePrintPreviewDialog(rfh->GetRoutingID()));
}

void PrintViewManagerQt::OnMetafileReadyForPrinting(
    const PrintHostMsg_DidPreviewDocument_Params& params)
{
    StopWorker(params.document_cookie);

    // Create local copies so we can reset the state and take a new pdf print job.
    base::Callback<void(const std::vector<char>&)> pdf_print_callback = m_pdfPrintCallback;
    base::Callback<void(bool)> pdf_save_callback = m_pdfSaveCallback;
    base::FilePath pdfOutputPath = m_pdfOutputPath;

    resetPdfState();

    if (!pdf_print_callback.is_null()) {
        std::vector<char> data_vector = GetStdVectorFromHandle(params.metafile_data_handle,
                                                               params.data_size);
        content::BrowserThread::PostTask(content::BrowserThread::UI,
                                         FROM_HERE,
                                         base::Bind(pdf_print_callback, data_vector));
    } else {
        scoped_refptr<base::RefCountedBytes> data_bytes
                = GetBytesFromHandle(params.metafile_data_handle, params.data_size);
        content::BrowserThread::PostTask(content::BrowserThread::FILE,
               FROM_HERE,
               base::Bind(&SavePdfFile, data_bytes, pdfOutputPath, pdf_save_callback));
    }
}

void PrintViewManagerQt::OnDidShowPrintDialog()
{
}

// content::WebContentsObserver implementation.
void PrintViewManagerQt::DidStartLoading()
{
}

// content::WebContentsObserver implementation.
// Cancels the print job.
void PrintViewManagerQt::NavigationStopped()
{
    if (!m_pdfPrintCallback.is_null()) {
        content::BrowserThread::PostTask(content::BrowserThread::UI,
                                         FROM_HERE,
                                         base::Bind(m_pdfPrintCallback, std::vector<char>()));
    }
    resetPdfState();
}

void PrintViewManagerQt::RenderProcessGone(base::TerminationStatus status)
{
    PrintViewManagerBaseQt::RenderProcessGone(status);
    if (!m_pdfPrintCallback.is_null()) {
        content::BrowserThread::PostTask(content::BrowserThread::UI,
                                         FROM_HERE,
                                         base::Bind(m_pdfPrintCallback, std::vector<char>()));
    }
    resetPdfState();
}


} // namespace QtWebEngineCore
