/*
 *      Copyright (C) 2012 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */


// XBMC
//
// libraries:
//   - CDRipX   : doesnt support section loading yet
//   - xbfilezilla : doesnt support section loading yet
//

#include "system.h"
#include "Application.h"
#include "PlayListPlayer.h"
#include "settings/AppParamParser.h"
#include "settings/AdvancedSettings.h"
#include "utils/log.h"

#if defined(TARGET_LINUX) && defined(DEBUG)
#include <sys/resource.h>
#include <signal.h>
#endif
#if defined(TARGET_DARWIN)
#include "Util.h"
#endif

extern int XBMC_Initialize(XBMC_PLATFORM *platform, int argc, const char** argv)
{
  g_application.PlatformInitialize(platform);

  //this can't be set from CAdvancedSettings::Initialize() because it will overwrite
  //the loglevel set with the --debug flag
#ifdef _DEBUG
  g_advancedSettings.m_logLevel     = LOG_LEVEL_DEBUG;
  g_advancedSettings.m_logLevelHint = LOG_LEVEL_DEBUG;
#else
  g_advancedSettings.m_logLevel     = LOG_LEVEL_NORMAL;
  g_advancedSettings.m_logLevelHint = LOG_LEVEL_NORMAL;
#endif
  CLog::SetLogLevel(g_advancedSettings.m_logLevel);

#ifdef TARGET_LINUX
#if defined(DEBUG)
  struct rlimit rlim;
  rlim.rlim_cur = rlim.rlim_max = RLIM_INFINITY;
  if (setrlimit(RLIMIT_CORE, &rlim) == -1)
    CLog::Log(LOGDEBUG, "Failed to set core size limit (%s)", strerror(errno));
#endif
  // Prevent child processes from becoming zombies on exit if not waited upon. See also Util::Command
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));

  sa.sa_flags = SA_NOCLDWAIT;
  sa.sa_handler = SIG_IGN;
  sigaction(SIGCHLD, &sa, NULL);
#endif
  setlocale(LC_NUMERIC, "C");
  g_advancedSettings.Initialize();
  
#ifndef TARGET_WINDOWS
  if ((platform->flags & XBMCRunPrimary) && argc > 0 && argv != NULL)
  {
    CAppParamParser appParamParser;
    appParamParser.Parse(argv, argc);
  }
#endif
  g_application.Preflight();
#if defined(TARGET_ANDROID)
  platform->android_printf("Creating application. Hello from Android!");
#endif

  if (!g_application.Create())
  {
    CLog::Log(LOGERROR, "ERROR: Unable to create application. Exiting");
    return 1;
  }
  return 0;
}

extern int XBMC_Run()
{
  return g_application.Run();
}
