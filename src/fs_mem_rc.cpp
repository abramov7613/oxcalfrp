/////////////////////////////////////////////////////////////////////////////
// this is edited 'wxWidgets/src/common/fs_mem.cpp' file
/////////////////////////////////////////////////////////////////////////////
#include "wx/wxprec.h"
#include "wx/mstream.h"
#include "fs_mem_rc.h"

// represents a file entry in wxMemoryFS
class wxMemoryFSFile
{
public:
  const char *m_Data;
  const size_t m_Len;
  const wxString m_MimeType;
  wxMemoryFSFile(const char *data, size_t len, const wxString& mime)
  : m_Data(data), m_Len(len), m_MimeType(mime) {}
  virtual ~wxMemoryFSFile(){}
private:
  wxDECLARE_NO_COPY_CLASS(wxMemoryFSFile);
};

//--------------------------------------------------------------------------------
// wxMemoryFSRCHandler
//--------------------------------------------------------------------------------

wxMemoryFSHash wxMemoryFSRCHandler::m_Hash;


wxMemoryFSRCHandler::wxMemoryFSRCHandler() : wxFileSystemHandler()
{
}

wxMemoryFSRCHandler::~wxMemoryFSRCHandler()
{
  // as only one copy of FS handler is supposed to exist, we may silently
  // delete static data here. (There is no way how to remove FS handler from
  // wxFileSystem other than releasing _all_ handlers.)
  WX_CLEAR_HASH_MAP(wxMemoryFSHash, m_Hash);
}

bool wxMemoryFSRCHandler::CanOpen(const wxString& location)
{
  return GetProtocol(location) == "memory";
}

wxFSFile * wxMemoryFSRCHandler::OpenFile(wxFileSystem& WXUNUSED(fs),
                                          const wxString& location)
{
	wxMemoryFSHash::const_iterator i = m_Hash.find(GetRightLocation(location));
	if ( i == m_Hash.end() )
  	return NULL;

	const wxMemoryFSFile * const obj = i->second;

	return new wxFSFile
          		(
            		new wxMemoryInputStream(obj->m_Data, obj->m_Len),
            		location,
            		obj->m_MimeType,
            		GetAnchor(location),
            		wxDateTime::Now()
          		);
}

wxString wxMemoryFSRCHandler::FindFirst(const wxString& url, int flags)
{
  // Make sure to reset the find iterator, so that calling FindNext() doesn't
  // reuse its value from the last search that could well be invalid.
  m_findIter = m_Hash.end();

  if ( (flags & wxDIR) && !(flags & wxFILE) )
  {
      // we only store files, not directories, so we don't risk finding
      // anything
      return wxString();
  }

  const wxString spec = GetRightLocation(url);
  if ( spec.find_first_of("?*") == wxString::npos )
  {
      // simple case: there are no wildcard characters so we can return
      // either 0 or 1 results and we can find the potential match quickly
      return m_Hash.count(spec) ? url : wxString();
  }
  //else: deal with wildcards in FindNext()

  m_findArgument = spec;
  m_findIter = m_Hash.begin();

  return FindNext();
}

wxString wxMemoryFSRCHandler::FindNext()
{
  while ( m_findIter != m_Hash.end() )
  {
      const wxString& path = m_findIter->first;

      // advance m_findIter first as we need to do it anyhow, whether it
      // matches or not
      ++m_findIter;

      if ( path.Matches(m_findArgument) )
          return "memory:" + path;
  }

  return wxString();
}

bool wxMemoryFSRCHandler::CheckDoesntExist(const wxString& filename)
{
  if ( m_Hash.count(filename) )
  {
      //wxLogError(_("Memory VFS already contains file '%s'!"), filename);
      return false;
  }

  return true;
}


/*static*/
void wxMemoryFSRCHandler::AddFileWithMimeType(const wxString& filename,
                                              const wxString& textdata,
                                              const wxString& mimetype)
{
  // We try to use the provided data "as is" if possible, but if not, we fall
  // back to UTF-8 because it's better to do this than just lose the data
  // completely.
  wxCharBuffer buf(textdata.To8BitData());
  if ( !buf.length() )
      buf = textdata.utf8_str();

  AddFileWithMimeType(filename, buf.data(), buf.length(), mimetype);
}


/*static*/
void wxMemoryFSRCHandler::AddFileWithMimeType(const wxString& filename,
                                              const void *binarydata, size_t size,
                                              const wxString& mimetype)
{
  if ( !CheckDoesntExist(filename) )
      return;

  m_Hash[filename] = new wxMemoryFSFile(static_cast<const char*>(binarydata), size, mimetype);
}

/*static*/
void wxMemoryFSRCHandler::AddFile(const wxString& filename,
                                  const wxString& textdata)
{
  AddFileWithMimeType(filename, textdata, wxEmptyString);
}


/*static*/
void wxMemoryFSRCHandler::AddFile(const wxString& filename,
                                  const void *binarydata, size_t size)
{
  AddFileWithMimeType(filename, binarydata, size, wxEmptyString);
}



/*static*/ void wxMemoryFSRCHandler::RemoveFile(const wxString& filename)
{
  wxMemoryFSHash::iterator i = m_Hash.find(filename);
  if ( i == m_Hash.end() )
  {
      //wxLogError(_("Trying to remove file '%s' from memory VFS, "
      //              "but it is not loaded!"),
      //            filename);
      return;
  }

  delete i->second;
  m_Hash.erase(i);
}
