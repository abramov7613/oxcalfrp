#include <wx/wx.h>
#include <wx/app.h>
#include <wx/cmdline.h>
#include <wx/string.h>
#include <wx/tokenzr.h>
#include <wx/arrstr.h>
#include <wx/wfstream.h>
#include <wx/zipstrm.h>
#include <wx/filename.h>

static const wxCmdLineEntryDesc cmdLineDesc[] =
{
	{ wxCMD_LINE_SWITCH, "h", "help", "show this help message",
			wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
	{ wxCMD_LINE_OPTION, "i", "input", "input files list, separated with ';'",
			wxCMD_LINE_VAL_STRING, wxCMD_LINE_OPTION_MANDATORY },
	{ wxCMD_LINE_OPTION, "o", "output", "output file name",
			wxCMD_LINE_VAL_STRING, wxCMD_LINE_OPTION_MANDATORY },
	// ... your other command line options here...
	wxCMD_LINE_DESC_END
};

int main(int argc, char **argv)
{
	wxInitializer initializer;
	if ( !initializer )
	{
			fprintf(stderr, "Failed to initialize the wxWidgets library, aborting.");
			return -1;
	}
	wxArrayString in_files;
	wxString out_file;
	wxCmdLineParser parser(cmdLineDesc, argc, argv);
	if ( parser.Parse()==0 )
	{
		wxString lst;
		if (parser.Found("i", &lst)) {
			wxStringTokenizer tokenizer(lst, ";");
			while(tokenizer.HasMoreTokens()) { in_files.Add(tokenizer.GetNextToken()); }
		}
		lst.Clear();
		if (parser.Found("o", &lst)) {
			out_file = lst;
		}
	}
	if(!in_files.empty() && !out_file.empty()) {
		wxFileOutputStream out(wxFileName(out_file).GetAbsolutePath());
		if(!out.IsOk()) {
				wxPrintf("Write file error: %s\n", out_file);
				return -1;
		}
		wxZipOutputStream zip(out);
		for(const auto& f: in_files) {
			wxFileInputStream in_stream(wxFileName(f).GetAbsolutePath());
			if(in_stream.IsOk()) {
				zip.PutNextEntry(wxFileName(f).GetFullName());
				in_stream.Read(zip);
			}
			if(!in_stream.Eof() || out.GetLastError()!=wxSTREAM_NO_ERROR) {
				wxPrintf("Read file error: %s\n", f);
				return -1;
			}
		}
	}
	if ( argc == 1 )
	{
			// If there were no command-line options supplied, emit a message
			// otherwise it's not obvious that the sample ran successfully
			wxPrintf("Simple wxWidgets-based console zip archiver\n");
			wxPrintf("For more information, run it again with the --help option\n");
	}
	return 0;
}
