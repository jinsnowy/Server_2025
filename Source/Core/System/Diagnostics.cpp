#include "stdafx.h"
#include "Diagnostics.h"
#include "DateTime.h"

#include "Core/System/Path.h"

namespace System {
	std::string Diagnostics::GetCurrentWorkingDirectory() {
		return System::Path::GetWokringDirectory();
	}

	std::string Diagnostics::GetProgramName() {
		auto executable_path = System::Path::GetExecutablePath();
		return System::Path::GetFileName(executable_path);
	}

}