//===-- PlatformRemoteiOS.cpp -----------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "PlatformRemoteiOS.h"

// C Includes
// C++ Includes
// Other libraries and framework includes
// Project includes
#include "lldb/Breakpoint/BreakpointLocation.h"
#include "lldb/Core/Module.h"
#include "lldb/Core/ModuleList.h"
#include "lldb/Core/ModuleSpec.h"
#include "lldb/Core/PluginManager.h"
#include "lldb/Host/Host.h"
#include "lldb/Target/Process.h"
#include "lldb/Target/Target.h"
#include "lldb/Utility/ArchSpec.h"
#include "lldb/Utility/FileSpec.h"
#include "lldb/Utility/Log.h"
#include "lldb/Utility/Status.h"
#include "lldb/Utility/StreamString.h"

using namespace lldb;
using namespace lldb_private;

//------------------------------------------------------------------
// Static Variables
//------------------------------------------------------------------
static uint32_t g_initialize_count = 0;

//------------------------------------------------------------------
// Static Functions
//------------------------------------------------------------------
void PlatformRemoteiOS::Initialize() {
  PlatformDarwin::Initialize();

  if (g_initialize_count++ == 0) {
    PluginManager::RegisterPlugin(PlatformRemoteiOS::GetPluginNameStatic(),
                                  PlatformRemoteiOS::GetDescriptionStatic(),
                                  PlatformRemoteiOS::CreateInstance);
  }
}

void PlatformRemoteiOS::Terminate() {
  if (g_initialize_count > 0) {
    if (--g_initialize_count == 0) {
      PluginManager::UnregisterPlugin(PlatformRemoteiOS::CreateInstance);
    }
  }

  PlatformDarwin::Terminate();
}

PlatformSP PlatformRemoteiOS::CreateInstance(bool force, const ArchSpec *arch) {
  Log *log(GetLogIfAllCategoriesSet(LIBLLDB_LOG_PLATFORM));
  if (log) {
    const char *arch_name;
    if (arch && arch->GetArchitectureName())
      arch_name = arch->GetArchitectureName();
    else
      arch_name = "<null>";

    const char *triple_cstr =
        arch ? arch->GetTriple().getTriple().c_str() : "<null>";

    log->Printf("PlatformRemoteiOS::%s(force=%s, arch={%s,%s})", __FUNCTION__,
                force ? "true" : "false", arch_name, triple_cstr);
  }

  bool create = force;
  if (create == false && arch && arch->IsValid()) {
    switch (arch->GetMachine()) {
    case llvm::Triple::arm:
    case llvm::Triple::aarch64:
    case llvm::Triple::thumb: {
      const llvm::Triple &triple = arch->GetTriple();
      llvm::Triple::VendorType vendor = triple.getVendor();
      switch (vendor) {
      case llvm::Triple::Apple:
        create = true;
        break;

      default:
        break;
      }
      if (create) {
        switch (triple.getOS()) {
        case llvm::Triple::Darwin: // Deprecated, but still support Darwin for
                                   // historical reasons
        case llvm::Triple::IOS:    // This is the right triple value for iOS
                                   // debugging
          break;

        default:
          create = false;
          break;
        }
      }
    } break;
    default:
      break;
    }
  }

  if (create) {
    if (log)
      log->Printf("PlatformRemoteiOS::%s() creating platform", __FUNCTION__);

    return lldb::PlatformSP(new PlatformRemoteiOS());
  }

  if (log)
    log->Printf("PlatformRemoteiOS::%s() aborting creation of platform",
                __FUNCTION__);

  return lldb::PlatformSP();
}

lldb_private::ConstString PlatformRemoteiOS::GetPluginNameStatic() {
  static ConstString g_name("remote-ios");
  return g_name;
}

const char *PlatformRemoteiOS::GetDescriptionStatic() {
  return "Remote iOS platform plug-in.";
}

//------------------------------------------------------------------
/// Default Constructor
//------------------------------------------------------------------
PlatformRemoteiOS::PlatformRemoteiOS()
    : PlatformRemoteDarwinDevice() {}

bool PlatformRemoteiOS::GetSupportedArchitectureAtIndex(uint32_t idx,
                                                        ArchSpec &arch) {
  return ARMGetSupportedArchitectureAtIndex(idx, arch);
}


void PlatformRemoteiOS::GetDeviceSupportDirectoryNames (std::vector<std::string> &dirnames) 
{
    dirnames.clear();
    dirnames.push_back("iOS DeviceSupport");
}

std::string PlatformRemoteiOS::GetPlatformName ()
{
    return "iPhoneOS.platform";
}
