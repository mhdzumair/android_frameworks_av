// #include <MtpTypes.h>
#include <system/audio.h>
//#include <StrongPointer.h>
#include <media/IAudioFlinger.h>
#include <hardware/audio.h>
#include <stdlib.h>
#include <dlfcn.h>

#ifndef __unused
#define __unused  __attribute__((__unused__))
#endif

namespace android {

//typedef void (*audio_error_callback)(status_t err);

class AudioSystem
{
public:
    static bool getVoiceUnlockDLInstance();
    static int GetVoiceUnlockDLLatency();
    static int SetVoiceUnlockSRC(uint outSR, uint outChannel);
    static bool stopVoiceUnlockDL();
    static bool startVoiceUnlockDL();
    static int ReadRefFromRing(void* buf, uint32_t datasz, void* DLtime);
    static int GetVoiceUnlockULTime(void* DLtime);
    static void freeVoiceUnlockDLInstance();

}; // class

bool AudioSystem::getVoiceUnlockDLInstance()
{
  return false;
}

int AudioSystem::GetVoiceUnlockDLLatency()
{
  return 0;
}

int AudioSystem::SetVoiceUnlockSRC(uint outSR __unused, uint outChannel __unused)
{
  return 0;
}

bool AudioSystem::stopVoiceUnlockDL()
{
  return false;
}

bool AudioSystem::startVoiceUnlockDL()
{
  return false;
}

int AudioSystem::ReadRefFromRing(void* buf __unused, uint32_t datasz __unused, void* DLtime __unused)
{
  return 0;
}

int AudioSystem::GetVoiceUnlockULTime(void* DLtime __unused)
{
  return 0;
}

void AudioSystem::freeVoiceUnlockDLInstance() {}

} // namespace
