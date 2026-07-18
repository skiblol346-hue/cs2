═══════════════════════════════════════════════════════════════════════════
  VacLiveBypass Kernel Injection Kit — NLinjector-Style Standalone
═══════════════════════════════════════════════════════════════════════════

Standalone launcher (PowerShell script with a batch wrapper) that reproduces
the complete NLinjector(2).exe pipeline. Place all files next to the .bat file,
double-click, and everything runs automatically.

What Launch-Inject.bat does (1:1 with NLinjector, verified in IDA):

  Step 1   Verify administrator privileges.
  Step 2   Disable VulnerableDriverBlocklistEnable in the registry
             (Windows 11 22H2+)
             HKLM\System\...\CI\Config\VulnerableDriverBlocklistEnable = 0
             (Equivalent to NLinjector's sub_14000D050 when build >= 22621)
  Step 3   Wait for cs2.exe (poll every 1 second until detected).
  Step 4   Compute client.dll SHA hash and select the matching
             fva_recon_<depot>.dll.
  Step 4.5 Automatically close MSI Afterburner, RTSS, and Encoder Server
             (they keep RTCore64 loaded).
             Wait up to 15 seconds for the RTCore64 service to reach
             the Stopped state.
  Step 5   Install the code-signing certificate into Trusted Root and
             Trusted Publisher.
  Step 6   Clean up orphaned KDU provider services (RTCore64, NalDrv, etc.)
             with automatic fallback to provider 0 if RTCore64 remains
             stuck in StopPending.
  Step 7   Run:
               kdu.exe -prv 1 -dse 0
             (RTCore64 exploit → patch g_CiOptions in CI.dll)
             Fallback:
               kdu.exe -prv 0 -dse 0
             (Intel NAL provider)
  Step 8   Create and start the CS2HexSyncCompatSvc service
             (signed driver).
  Step 9   Open \\.\CS2HexSyncCompat and issue the SELF_TEST IOCTL
             (0x200008421).
  Step 10  Launch CS2UnifiedInjector using the kernel method:
             10a. BypassUserHooks — bypass 17 inline API hooks
                  (kernel32 / ntdll / KernelBase)
             10b. ALLOCATE_MEMORY (IOCTL 0x222018) — allocate remote memory
             10c. WRITE_MEMORY (0x222004) — write DLL path remotely
             10d. READ_MEMORY (0x222000) — verify the write
             10e. GET_MODULE_BASE (0x222008) — locate remote kernel32.dll
             10f. CREATE_THREAD (0x222020) — invoke remote LoadLibraryW
             10g. Poll GET_MODULE_BASE for up to 5 seconds to verify
                  successful DLL loading
             10h. RestoreUserHooks — restore original hook bytes
  Step 11  Run:
               kdu.exe -prv <provider> -dse 6
             to restore DSE.
  Step 12  Stop and remove the temporary service.

Each step is color-coded:
  [+] Green  = Success
  [!] Yellow = Warning
  [-] Red    = Error

Any failure immediately stops the pipeline with a clear explanation of the
problem and the required corrective action.

═══════════════════════════════════════════════════════════════════════════
  Folder Contents
═══════════════════════════════════════════════════════════════════════════

CS2HexSyncCompatDriver.sys   16688 B    Signed HexSync driver reimplementation.
                                         Implements the same 9 IOCTLs as the
                                         original.

CS2HexSyncCompatCert.cer       820 B    Public certificate used for installation.

CS2UnifiedInjector.exe      273408 B    Injector with 17-hook bypass and
                                         injection verification.

kdu.exe                    2477568 B    Standalone KDU
                                         (65 embedded providers).

drv64.dll                  2045952 B    Tanikaze database backup
                                         (KDUEXT fallback).

fva_recon_24134959.dll       82944 B    Payload for the current live depot.

fva_recon_14167.dll          83456 B    Payload for the baseline depot.

Launch-Inject.ps1            ~19 KB     Main PowerShell pipeline
                                         (PowerShell 5.1+).

Launch-Inject.bat             455 B     Batch wrapper with automatic
                                         UAC elevation.

README.txt                              This document.

═══════════════════════════════════════════════════════════════════════════
  Running
═══════════════════════════════════════════════════════════════════════════

Simply double-click Launch-Inject.bat.

Options (passed directly to Launch-Inject.ps1):

  -Dll fva_recon_14167.dll
      Manually select the payload DLL.

  -KduProvider 0
      Use the Intel NAL provider instead of RTCore64.

  -WithFallback
      Allow automatic fallback from provider 1 to provider 0.

  -NoDbg
      Minimal console output.

═══════════════════════════════════════════════════════════════════════════
  Requirements
═══════════════════════════════════════════════════════════════════════════

- Windows 10/11 x64
- Administrator privileges
- HVCI (Memory Integrity) must be disabled
    Settings → Windows Security → Device Security →
    Core Isolation → Memory Integrity → Off

    Otherwise the RTCore64 driver will be blocked.
    In that case, use -KduProvider 0 (Intel NAL).

- Test Signing is NOT required
    DSE is disabled temporarily through the exploit.

- MSI Afterburner is automatically closed by the launcher
    because RTCore64 cannot be restarted while already in use.

═══════════════════════════════════════════════════════════════════════════
  DLL Automation (fva_recon)
═══════════════════════════════════════════════════════════════════════════

After the DLL is injected into CS2:

- Logging is disabled by default (retail build).

  Enable logging by either:

      set FVA_DEBUG=1

  or creating the file:

      C:\vmp\.fva_debug

  Logs are written to:

      C:\vmp\fva_recon.log

═══════════════════════════════════════════════════════════════════════════
  SHA-256 (v1.4 Final)
═══════════════════════════════════════════════════════════════════════════

CS2HexSyncCompatDriver.sys   2f2cf4d01e49544ede5e9e4b3b781286653ddcd561938dab4ef77b832083771c
CS2UnifiedInjector.exe       a1387d1ad26d922a831733897a5013ba0a2d53faaf306782bce0e5a75b25ea07
kdu.exe                      3bce7d1416737b43a4a6ffcdca68bc5f5696a5ae023b0ca5218db56244998c0e
drv64.dll                    3d02a47946cbac1723caed97ce96c8a585d4fc4a60fbe2d7fcbe54af62e88c17
fva_recon_24134959.dll       bdc1f31a7b598728375e56eb0814fdff7e225f1a2500994a469ed75af77b5992
fva_recon_14167.dll          859fcc0b012c975c22feb85dc6d1b83605e454529daa30c7b1c7b1be41ffa6f8
CS2HexSyncCompatCert.cer     9e514db1cf3d0a79fed78b0ba4ab4d47ba040ed3f4696e911158a070c59944f0

═══════════════════════════════════════════════════════════════════════════
  Legal
═══════════════════════════════════════════════════════════════════════════

For personal research and educational use only. Not intended for use on live
Valve servers. Using this software in matchmaking or competitive environments
violates the Steam Subscriber Agreement and Valve's Terms of Service.