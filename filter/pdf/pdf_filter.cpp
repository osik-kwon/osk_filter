#include "filter_pch.h"
#include "pdf/pdf_filter.h"
#include "locale/charset_encoder.h"
#include "io/file_stream.h"

#include <public/fpdf_text.h>
#include <public/fpdfview.h>

#include <fstream>
#include <string>

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/newline.hpp>
#include <boost/iostreams/device/array.hpp>

#include <public/fpdf_dataavail.h>
#include <public/fpdf_ext.h>
#include <public/fpdf_text.h>
#include <public/fpdf_formfill.h>
#include <pdf/core/include/fpdftext/fpdf_text.h>
//#include "testing/gtest/include/gtest/gtest.h"
//#include "v8/include/v8.h"

class TestLoader;

// This class is used to load a PDF document, and then run programatic
// API tests against it.
class EmbedderTest : //public ::testing::Test,
    public UNSUPPORT_INFO,
    public IPDF_JSPLATFORM,
    public FPDF_FORMFILLINFO {
public:
    class Delegate {
    public:
        virtual ~Delegate() { }

        // Equivalent to UNSUPPORT_INFO::FSDK_UnSupport_Handler().
        virtual void UnsupportedHandler(int type) { }

        // Equivalent to IPDF_JSPLATFORM::app_alert().
        virtual int Alert(FPDF_WIDESTRING message, FPDF_WIDESTRING title,
            int type, int icon) {
            return 0;
        }

        // Equivalent to FPDF_FORMFILLINFO::FFI_SetTimer().
        virtual int SetTimer(int msecs, TimerCallback fn) { return 0; }

        // Equivalent to FPDF_FORMFILLINFO::FFI_KillTimer().
        virtual void KillTimer(int id) { }
    };

    EmbedderTest();
    virtual ~EmbedderTest();

    void SetUp();//override;
    void TearDown(); //override;

    void SetDelegate(Delegate* delegate) {
        delegate_ = delegate ? delegate : default_delegate_;
    }

    FPDF_DOCUMENT document() { return document_; }
    FPDF_FORMHANDLE form_handle() { return form_handle_; }

    // Open the document specified by |filename|, and create its form fill
    // environment, or return false on failure.
    virtual bool OpenDocument(const std::string& filename);

    // Perform JavaScript actions that are to run at document open time.
    virtual void DoOpenActions();

    // Determine the page numbers present in the document.
    virtual int GetFirstPageNum();
    virtual int GetPageCount();

    // Load a specific page of the open document.
    virtual FPDF_PAGE LoadPage(int page_number);

    // Convert a loaded page into a bitmap.
    virtual FPDF_BITMAP RenderPage(FPDF_PAGE page);

    // Relese the resources obtained from LoadPage(). Further use of |page|
    // is prohibited after this call is made.
    virtual void UnloadPage(FPDF_PAGE page);

protected:
    Delegate* delegate_;
    Delegate* default_delegate_;
    FPDF_DOCUMENT document_;
    FPDF_FORMHANDLE form_handle_;
    FPDF_AVAIL avail_;
    FX_DOWNLOADHINTS hints_;
    FPDF_FILEACCESS file_access_;
    FX_FILEAVAIL file_avail_;
    //v8::Platform* platform_;
    //v8::StartupData natives_;
    //v8::StartupData snapshot_;
    TestLoader* loader_;

    std::vector<char> contents;

private:
    static void UnsupportedHandlerTrampoline(UNSUPPORT_INFO*, int type);
    static int AlertTrampoline(IPDF_JSPLATFORM* plaform, FPDF_WIDESTRING message,
        FPDF_WIDESTRING title, int type, int icon);
    static int SetTimerTrampoline(FPDF_FORMFILLINFO* info, int msecs,
        TimerCallback fn);
    static void KillTimerTrampoline(FPDF_FORMFILLINFO* info, int id);
};

// Copyright (c) 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <list>
#include <string>
#include <utility>
#include <vector>

#include <public/fpdf_text.h>
#include <public/fpdfview.h>
//#include "testing/gmock/include/gmock/gmock.h"
//#include "v8/include/libplatform/libplatform.h"
//#include "v8/include/v8.h"

#ifdef _WIN32
#define snprintf _snprintf
#define PATH_SEPARATOR '\\'
#else
#define PATH_SEPARATOR '/'
#endif

namespace {

    const char* g_exe_path_ = nullptr;

    static std::vector<char> GetFileContents(const std::string& filename)
    {
        // open the file:
        std::ifstream file(filter::to_fstream_path(filename), std::ios::binary);

        // Stop eating new lines in binary mode
        file.unsetf(std::ios::skipws);

        // get its size:
        std::streampos fileSize;

        file.seekg(0, std::ios::end);
        fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        // reserve capacity
        std::vector<char> vec;
        vec.reserve(fileSize);

        // read the data:
        vec.insert(vec.begin(),
            std::istream_iterator<BYTE>(file),
            std::istream_iterator<BYTE>());

        return vec;
    }

    /*
    // Reads the entire contents of a file into a newly malloc'd buffer.
    static char* GetFileContents(const char* filename, size_t* retlen) {
        FILE* file = fopen(filename, "rb");
        if (!file) {
            fprintf(stderr, "Failed to open: %s\n", filename);
            return nullptr;
        }
        (void)fseek(file, 0, SEEK_END);
        size_t file_length = ftell(file);
        if (!file_length) {
            return nullptr;
        }
        (void)fseek(file, 0, SEEK_SET);
        char* buffer = (char*)malloc(file_length);
        if (!buffer) {
            return nullptr;
        }
        size_t bytes_read = fread(buffer, 1, file_length, file);
        (void)fclose(file);
        if (bytes_read != file_length) {
            fprintf(stderr, "Failed to read: %s\n", filename);
            free(buffer);
            return nullptr;
        }
        *retlen = bytes_read;
        return buffer;
    }
    */

#ifdef V8_USE_EXTERNAL_STARTUP_DATA
    // Returns the full path for an external V8 data file based on either
    // the currect exectuable path or an explicit override.
    static std::string GetFullPathForSnapshotFile(const std::string& exe_path,
        const std::string& filename) {
        std::string result;
        if (!exe_path.empty()) {
            size_t last_separator = exe_path.rfind(PATH_SEPARATOR);
            if (last_separator != std::string::npos) {
                result = exe_path.substr(0, last_separator + 1);
            }
        }
        result += filename;
        return result;
    }

    // Reads an extenal V8 data file from the |options|-indicated location,
    // returing true on success and false on error.
    static bool GetExternalData(const std::string& exe_path,
        const std::string& filename,
        v8::StartupData* result_data) {
        std::string full_path = GetFullPathForSnapshotFile(exe_path, filename);
        size_t data_length = 0;
        char* data_buffer = GetFileContents(full_path.c_str(), &data_length);
        if (!data_buffer) {
            return false;
        }
        result_data->data = const_cast<const char*>(data_buffer);
        result_data->raw_size = data_length;
        return true;
    }
#endif  // V8_USE_EXTERNAL_STARTUP_DATA

}  // namespace

class TestLoader {
public:
    TestLoader(const char* pBuf, size_t len);

    const char* m_pBuf;
    size_t m_Len;
};

TestLoader::TestLoader(const char* pBuf, size_t len)
    : m_pBuf(pBuf), m_Len(len) {
}

int Get_Block(void* param, unsigned long pos, unsigned char* pBuf,
    unsigned long size) {
    TestLoader* pLoader = (TestLoader*)param;
    if (pos + size < pos || pos + size > pLoader->m_Len) return 0;
    memcpy(pBuf, pLoader->m_pBuf + pos, size);
    return 1;
}

FPDF_BOOL Is_Data_Avail(FX_FILEAVAIL* pThis, size_t offset, size_t size) {
    return true;
}

void Add_Segment(FX_DOWNLOADHINTS* pThis, size_t offset, size_t size) {
}

EmbedderTest::EmbedderTest() :
    document_(nullptr),
    form_handle_(nullptr),
    avail_(nullptr),
    loader_(nullptr)
{
    memset(&hints_, 0, sizeof(hints_));
    memset(&file_access_, 0, sizeof(file_access_));
    memset(&file_avail_, 0, sizeof(file_avail_));
    default_delegate_ = new EmbedderTest::Delegate();
    delegate_ = default_delegate_;
    SetUp();
}

EmbedderTest::~EmbedderTest()
{
    TearDown();
    delete default_delegate_;
}

void EmbedderTest::SetUp() {
    // v8::V8::InitializeICU();

     //platform_ = v8::platform::CreateDefaultPlatform();
     //v8::V8::InitializePlatform(platform_);
     //v8::V8::Initialize();

     // By enabling predictable mode, V8 won't post any background tasks.
     //const char predictable_flag[] = "--predictable";
     //v8::V8::SetFlagsFromString(predictable_flag,
     //                           static_cast<int>(strlen(predictable_flag)));

#ifdef V8_USE_EXTERNAL_STARTUP_DATA
    ASSERT_TRUE(GetExternalData(g_exe_path_, "natives_blob.bin", &natives_));
    ASSERT_TRUE(GetExternalData(g_exe_path_, "snapshot_blob.bin", &snapshot_));
    v8::V8::SetNativesDataBlob(&natives_);
    v8::V8::SetSnapshotDataBlob(&snapshot_);
#endif  // V8_USE_EXTERNAL_STARTUP_DATA

    FPDF_InitLibrary();

    UNSUPPORT_INFO* info = static_cast<UNSUPPORT_INFO*>(this);
    memset(info, 0, sizeof(UNSUPPORT_INFO));
    info->version = 1;
    info->FSDK_UnSupport_Handler = UnsupportedHandlerTrampoline;
    FSDK_SetUnSpObjProcessHandler(info);
}

void EmbedderTest::TearDown() {
    if (document_) {
        FORM_DoDocumentAAction(form_handle_, FPDFDOC_AACTION_WC);
        FPDFDOC_ExitFormFillEnvironment(form_handle_);
        FPDF_CloseDocument(document_);
    }
    FPDFAvail_Destroy(avail_);
    FPDF_DestroyLibrary();
    //v8::V8::ShutdownPlatform();
    //delete platform_;
    delete loader_;
}

bool EmbedderTest::OpenDocument(const std::string& filename)
{
    contents = GetFileContents(filename);
    if (contents.empty())
        return false;

    loader_ = new TestLoader(contents.data(), contents.size());
    file_access_.m_FileLen = static_cast<unsigned long>(contents.size());
    file_access_.m_GetBlock = Get_Block;
    file_access_.m_Param = loader_;

    file_avail_.version = 1;
    file_avail_.IsDataAvail = Is_Data_Avail;

    hints_.version = 1;
    hints_.AddSegment = Add_Segment;

    avail_ = FPDFAvail_Create(&file_avail_, &file_access_);
    (void)FPDFAvail_IsDocAvail(avail_, &hints_);

    if (!FPDFAvail_IsLinearized(avail_)) {
        document_ = FPDF_LoadCustomDocument(&file_access_, nullptr);
    }
    else {
        document_ = FPDFAvail_GetDocument(avail_, nullptr);
    }

    (void)FPDF_GetDocPermissions(document_);
    (void)FPDFAvail_IsFormAvail(avail_, &hints_);

    IPDF_JSPLATFORM* platform = static_cast<IPDF_JSPLATFORM*>(this);
    memset(platform, 0, sizeof(IPDF_JSPLATFORM));
    platform->version = 2;
    platform->app_alert = AlertTrampoline;

    FPDF_FORMFILLINFO* formfillinfo = static_cast<FPDF_FORMFILLINFO*>(this);
    memset(formfillinfo, 0, sizeof(FPDF_FORMFILLINFO));
    formfillinfo->version = 1;
    formfillinfo->FFI_SetTimer = SetTimerTrampoline;
    formfillinfo->FFI_KillTimer = KillTimerTrampoline;
    formfillinfo->m_pJsPlatform = platform;

    form_handle_ = FPDFDOC_InitFormFillEnvironment(document_, formfillinfo);
    FPDF_SetFormFieldHighlightColor(form_handle_, 0, 0xFFE4DD);
    FPDF_SetFormFieldHighlightAlpha(form_handle_, 100);

    return true;
}

void EmbedderTest::DoOpenActions() {
    FORM_DoDocumentJSAction(form_handle_);
    FORM_DoDocumentOpenAction(form_handle_);
}

int EmbedderTest::GetFirstPageNum() {
    int first_page = FPDFAvail_GetFirstPageNum(document_);
    (void)FPDFAvail_IsPageAvail(avail_, first_page, &hints_);
    return first_page;
}

int EmbedderTest::GetPageCount() {
    int page_count = FPDF_GetPageCount(document_);
    for (int i = 0; i < page_count; ++i) {
        (void)FPDFAvail_IsPageAvail(avail_, i, &hints_);
    }
    return page_count;
}

FPDF_PAGE EmbedderTest::LoadPage(int page_number) {
    FPDF_PAGE page = FPDF_LoadPage(document_, page_number);
    if (!page) {
        return nullptr;
    }
    FORM_OnAfterLoadPage(page, form_handle_);
    FORM_DoPageAAction(page, form_handle_, FPDFPAGE_AACTION_OPEN);
    return page;
}

FPDF_BITMAP EmbedderTest::RenderPage(FPDF_PAGE page) {
    int width = static_cast<int>(FPDF_GetPageWidth(page));
    int height = static_cast<int>(FPDF_GetPageHeight(page));
    FPDF_BITMAP bitmap = FPDFBitmap_Create(width, height, 0);
    FPDFBitmap_FillRect(bitmap, 0, 0, width, height, 0xFFFFFFFF);
    FPDF_RenderPageBitmap(bitmap, page, 0, 0, width, height, 0, 0);
    FPDF_FFLDraw(form_handle_, bitmap, page, 0, 0, width, height, 0, 0);
    return bitmap;
}

void EmbedderTest::UnloadPage(FPDF_PAGE page) {
    FORM_DoPageAAction(page, form_handle_, FPDFPAGE_AACTION_CLOSE);
    FORM_OnBeforeClosePage(page, form_handle_);
    FPDF_ClosePage(page);
}

// static
void EmbedderTest::UnsupportedHandlerTrampoline(UNSUPPORT_INFO* info,
    int type) {
    EmbedderTest* test = static_cast<EmbedderTest*>(info);
    test->delegate_->UnsupportedHandler(type);
}

// static
int EmbedderTest::AlertTrampoline(IPDF_JSPLATFORM* platform,
    FPDF_WIDESTRING message,
    FPDF_WIDESTRING title,
    int type,
    int icon) {
    EmbedderTest* test = static_cast<EmbedderTest*>(platform);
    return test->delegate_->Alert(message, title, type, icon);
}

// static
int EmbedderTest::SetTimerTrampoline(FPDF_FORMFILLINFO* info,
    int msecs, TimerCallback fn) {
    EmbedderTest* test = static_cast<EmbedderTest*>(info);
    return test->delegate_->SetTimer(msecs, fn);
}

// static
void EmbedderTest::KillTimerTrampoline(FPDF_FORMFILLINFO* info, int id) {
    EmbedderTest* test = static_cast<EmbedderTest*>(info);
    return test->delegate_->KillTimer(id);
}

void FPDFText_GetText_impl(FPDF_TEXTPAGE text_page, int start, int count, CFX_WideString& out)
{
    if (!text_page) return ;
    IPDF_TextPage* textpage = (IPDF_TextPage*)text_page;

    if (start >= textpage->CountChars())
        return ;

    out = textpage->GetPageText(start, count);
    if (out.GetLength() > count)
        out = out.Left(count);
}

namespace filter
{
namespace pdf
{
    std::string filter_t::open(const std::string& path)
    {
        return path;
    }

	filter_t::sections_t filter_t::extract_all_texts(const std::string& path)
	{
		sections_t sections;
        try
        {
            EmbedderTest tester;
            tester.OpenDocument(path);

            auto page_count = tester.GetPageCount();
            for (int i = 0; i < page_count; ++i)
            {
                FPDF_PAGE page = tester.LoadPage(i);
                FPDF_TEXTPAGE textpage = FPDFText_LoadPage(page);

                int count_chars = FPDFText_CountChars(textpage);
                std::vector<unsigned short> buffer;
                buffer.resize(count_chars + 1);


                CFX_WideString wstring;
                std::wstring page_texts;
                FPDFText_GetText_impl(textpage, 0, count_chars, wstring);
                for (int j = 0; j < wstring.GetLength(); ++j)
                {
                    page_texts.push_back((wchar_t)wstring[j]);
                }
                /*
                int text_length = FPDFText_GetText(textpage, 0, count_chars, buffer.data());
                std::wstring page_texts;
                for (int i = 0; i < text_length; ++i)
                {
                    wchar_t text = static_cast<wchar_t>(FPDFText_GetUnicode(textpage, i));
                    page_texts.push_back(text);
                }
                */
                FPDFText_ClosePage(textpage);
                tester.UnloadPage(page);

                using namespace boost::iostreams;
                using namespace std;

                std::string u8_page_texts = to_utf8(page_texts);
                filtering_istream in;
                in.push(newline_filter(newline::posix));
                in.push(array_source(u8_page_texts.c_str(), u8_page_texts.size()));

                sections.push_back(section_t());
                section_t& section = sections.back();

                string line;
                while (getline(in, line))
                {
                    if (!line.empty() && line.back() == 0)
                        line.pop_back();
                    line.push_back('\n');
                    section.push_back(to_wchar(line));
                }
            }
        }
        catch (const std::exception& e)
        {
            std::cout << e.what();
        }

		return sections;
	}
}
}