/////////////////////////////////////////////////////////////////////////////
// this is edited 'wxWidgets/include/wx/fs_mem.h' file
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_FS_MEM_RC_H_
#define _WX_FS_MEM_RC_H_
#include "wx/defs.h"
#include "wx/filesys.h"
#include "wx/hashmap.h"

class wxMemoryFSFile;
WX_DECLARE_STRING_HASH_MAP(wxMemoryFSFile *, wxMemoryFSHash);

// ----------------------------------------------------------------------------
// wxMemoryFSRCHandler
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxMemoryFSRCHandler : public wxFileSystemHandler
{
public:
    wxMemoryFSRCHandler();
    virtual ~wxMemoryFSRCHandler();

    // Add file to list of files stored in memory. Stored data (bitmap, text or
    // raw data) will be copied into private memory stream and available under
    // name "memory:" + filename
    static void AddFile(const wxString& filename, const wxString& textdata);
    static void AddFile(const wxString& filename, const void *binarydata, size_t size);
    static void AddFileWithMimeType(const wxString& filename,
                                    const wxString& textdata,
                                    const wxString& mimetype);
    static void AddFileWithMimeType(const wxString& filename,
                                    const void *binarydata, size_t size,
                                    const wxString& mimetype);

    // Remove file from memory FS and free occupied memory
    static void RemoveFile(const wxString& filename);

    virtual bool CanOpen(const wxString& location) wxOVERRIDE;
    virtual wxFSFile* OpenFile(wxFileSystem& fs, const wxString& location) wxOVERRIDE;
    virtual wxString FindFirst(const wxString& spec, int flags = 0) wxOVERRIDE;
    virtual wxString FindNext() wxOVERRIDE;

protected:
    // check that the given file is not already present in m_Hash; logs an
    // error and returns false if it does exist
    static bool CheckDoesntExist(const wxString& filename);

    // the hash map indexed by the names of the files stored in the memory FS
    static wxMemoryFSHash m_Hash;

    // the file name currently being searched for, i.e. the argument of the
    // last FindFirst() call or empty string if FindFirst() hasn't been called
    // yet
    wxString m_findArgument;

    // iterator into m_Hash used by FindFirst/Next(), possibly m_Hash.end()
    wxMemoryFSHash::const_iterator m_findIter;
};

#endif // _WX_FS_MEM_RC_H_
