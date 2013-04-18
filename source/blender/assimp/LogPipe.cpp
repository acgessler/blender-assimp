/*
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Contributor(s): Alexander Gessler
 *
 * ***** END GPL LICENSE BLOCK *****
 */

/** \file blender/assimp/bassimp.cpp
 *  \ingroup assimp
 */

#include <cassert>
#include "LogPipe.h"

#include "../../extern/assimp/include/assimp/DefaultLogger.hpp"

extern "C"
{

#include "BKE_report.h"

}

using namespace Assimp;

namespace {

// custom assimp logging implementation
class BlenderLogger : public Logger
{
public:

	BlenderLogger(ReportList& reports) 
		: reports(reports)
	{

	}

	virtual ~BlenderLogger() {
	}

public:


	virtual bool attachStream(LogStream *pStream, unsigned int severity = Debugging | Err | Warn | Info)
	{
		// not implemented, not needed
		return false;
	}


	virtual bool detatchStream(LogStream *pStream, unsigned int severity = Debugging | Err | Warn | Info)
	{
		// not implemented, not needed
		return false;
	}

public:
	
	virtual void OnDebug(const char* message) 
	{	
		BKE_report(&reports,RPT_DEBUG,AddPrefix(message).c_str());
	}

	virtual void OnInfo(const char* message) 
	{
		BKE_report(&reports,RPT_INFO,AddPrefix(message).c_str());
	}

	
	virtual void OnWarn(const char* message) 
	{
		BKE_report(&reports,RPT_WARNING,AddPrefix(message).c_str());
	}

	
	virtual void OnError(const char* message) 
	{
		BKE_report(&reports,RPT_ERROR,AddPrefix(message).c_str());
	}

private:

	std::string AddPrefix(const std::string& p) {
		return "[assimp] " + p;
	}

private:

	ReportList& reports;
};

} // !anon

namespace bassimp {

LogPipe::LogPipe(ReportList* reports)
{
	if(reports) {
		DefaultLogger::set(new BlenderLogger(*reports));
	}
}


LogPipe::~LogPipe()
{
	kill();
}


void LogPipe::kill()
{
	DefaultLogger::set(NULL);
}

}
