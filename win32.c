#include <windows.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <dsound.h>
#include <math.h>

#include "handmade.h"

#pragma intrinsic(__rdtsc)

/*
    consult MSDN for all of these!

    TODO: add mouse input and movement
    TODO: save game locations
    TODO: getting a handle to our own executable file
    TODO: asset loading path
    TODO: threading (launch a thread)
    TODO: raw input (support multiple keyboards)
    TODO: sleep/timebegin period
    TODO: clipcursor() for multimonitor support
    TODO: fullscreen support
    TODO: WM_SETCURSOR (set visibility)
    TODO: querycancelautoplay
    TODO: WM_ACTIVATEAPP (for when we are not the active app)
    TODO: Blit speed improvements (bitblt)
    TODO: hardware acceleratoin (OpenGL or Direct3d or BOTH??)
    TODO: getkeyboardlayout support different keyboard layouts
*/

#define BPP 4
#define PI 3.14159265359

struct win32_offscreen_buffer {
    int width;
    int height;
    int pitch;
    void *memory;
    BITMAPINFO info;
};

struct win32_window_dimension {
    int width;
    int height;
};

struct win32_sound_output {
    int SamplesPerSecond;
    int ToneHz;
    int square_wave_period;
    int BytesPerSample;
    int SecondaryBufferSize;
    int16_t tone_volume;
    uint32_t running_sample_index;
};

static int global_running;
static int global_movement;
static struct win32_offscreen_buffer globalbackbuffer;
static LPDIRECTSOUNDBUFFER globalDSSecondaryBuffer;

struct win32_window_dimension win32_get_window_dimension(HWND Window)
{
    struct win32_window_dimension result;

    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    result.width = ClientRect.right - ClientRect.left;
    result.height = ClientRect.bottom - ClientRect.top;

    return result;
}

typedef HRESULT (WINAPI *direct_sound_create)(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter);

static void win32_init_direct_sound(HWND Window, int32_t SamplesPerSecond, int32_t BufferSize)
{
    HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");
    if (DSoundLibrary) {
        direct_sound_create DirectSoundCreate = (direct_sound_create)GetProcAddress(DSoundLibrary, "DirectSoundCreate");
        LPDIRECTSOUND DirectSound;
        if (SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0))) {
            WAVEFORMATEX WaveFormat;
            memset(&WaveFormat, 0, sizeof(WAVEFORMATEX));

            WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
            WaveFormat.nChannels = 2;
            WaveFormat.nSamplesPerSec = SamplesPerSecond;
            WaveFormat.wBitsPerSample = 16;
            WaveFormat.nBlockAlign = (WaveFormat.nChannels * WaveFormat.wBitsPerSample) / 8;
            WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec * WaveFormat.nBlockAlign;
            WaveFormat.cbSize = 0;

            if (SUCCEEDED(IDirectSound_SetCooperativeLevel(DirectSound, Window, DSSCL_PRIORITY))) {
                DSBUFFERDESC BufferDescription;
                memset(&BufferDescription, 0, sizeof(DSBUFFERDESC));
                BufferDescription.dwSize = sizeof(BufferDescription);
                BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;

                LPDIRECTSOUNDBUFFER PrimaryBuffer;
                if (SUCCEEDED(IDirectSound_CreateSoundBuffer(DirectSound, &BufferDescription, &PrimaryBuffer, 0))) {
                    if (SUCCEEDED(IDirectSoundBuffer_SetFormat(PrimaryBuffer, &WaveFormat))) {
                        // TODO: We have finally set the format!
                    } else { /* diagnostics */}
                } else { /* diagnostics */}
            } else {}
            DSBUFFERDESC BufferDescription;
            memset(&BufferDescription, 0, sizeof(DSBUFFERDESC));

            BufferDescription.dwSize = sizeof(BufferDescription);
            BufferDescription.dwFlags = 0;
            BufferDescription.dwBufferBytes = BufferSize;
            BufferDescription.lpwfxFormat = &WaveFormat;
            if (SUCCEEDED(IDirectSound_CreateSoundBuffer(DirectSound, &BufferDescription, &globalDSSecondaryBuffer, 0))) {
                // TODO
            }
        } else {/* diagnostics */ }
    } else { /* diagnostics */}
    FreeLibrary(DSoundLibrary);
}

static void win32_Update(HDC DeviceContext, struct win32_offscreen_buffer *buffer, int window_w, int window_h)
{
    StretchDIBits(DeviceContext,
                  0, 0, window_w, window_h,
                  0, 0, buffer->width, buffer->height,
                  buffer->memory, &buffer->info, DIB_RGB_COLORS, SRCCOPY);
}

static void win32_UpdateInPaint(HWND Window)
{

    PAINTSTRUCT Paint;
    HDC DeviceContext = BeginPaint(Window, &Paint);

    struct win32_window_dimension dim = win32_get_window_dimension(Window);
    win32_Update(DeviceContext, &globalbackbuffer, dim.width, dim.height);
    EndPaint(Window, &Paint);
}

static void win32_ResizeDIBsect(struct win32_offscreen_buffer *buffer, int width, int height)
{
    int bitmap_memsize;

    if (buffer->memory) {
        VirtualFree(buffer->memory, 0, MEM_RELEASE);
    }

    buffer->width = width;
    buffer->height = height;

    buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
    buffer->info.bmiHeader.biWidth = buffer->width;
    buffer->info.bmiHeader.biHeight = -buffer->height;
    buffer->info.bmiHeader.biPlanes = 1;
    buffer->info.bmiHeader.biBitCount = 32;
    buffer->info.bmiHeader.biCompression = BI_RGB;

    bitmap_memsize = (buffer->width*buffer->height)*BPP;

    buffer->memory = VirtualAlloc(0, bitmap_memsize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    // TODO: windows page size is 4095, memsize / 4095 = n, n * 4095 = BitmapMemory
    // TODO: probably need to clear this to black!

    buffer->pitch = buffer->width*BPP;
}

void win32_keyboard_input(WPARAM WParam, LPARAM LParam)
{
    uint32_t VKCode = WParam;
    int wasdown = ((LParam & (1 << 30)) != 0);
    int isdown = ((LParam & (1 << 31)) == 0);

    if (wasdown != isdown) {
        if (VKCode == 'W') {}
        else if (VKCode == 'A') {}
        else if (VKCode == 'S') {}
        else if (VKCode == 'D') {}
        else if (VKCode == 'Q') {}
        else if (VKCode == 'E') {}

        else if (VKCode == VK_UP)     {global_movement = VK_UP;}
        else if (VKCode == VK_DOWN)   {global_movement = VK_DOWN;}
        else if (VKCode == VK_LEFT)   {global_movement = VK_LEFT;}
        else if (VKCode == VK_RIGHT)  {global_movement = VK_RIGHT;}
        else if (VKCode == VK_ESCAPE) {global_running = 0;}

        else if (VKCode == VK_SPACE) {}

        int alt_key_down = (LParam & (1 << 29));
        if ((VKCode == VK_F4)  && alt_key_down) {
            global_running = 0;
        }
    }
}

LRESULT CALLBACK WindowProc(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
    LRESULT result = 0;
    switch(Message)
    {
        case WM_SIZE:
            break;

        case WM_CLOSE:
            PostQuitMessage(0);
            break;

        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
            win32_keyboard_input(WParam, LParam);
            break;

        case WM_ACTIVATEAPP:
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        case WM_PAINT:
            win32_UpdateInPaint(Window);
            break;

        default:
            result = DefWindowProc(Window, Message, WParam, LParam);
            break;
    }
    return result;
}

static void win32_fill_sound_buffer(struct win32_sound_output *soundout, DWORD ByteToLock, DWORD BytesToWrite)
{

    VOID *R01;
    VOID *R02;
    DWORD R01_size;
    DWORD R02_size;

    if (SUCCEEDED(IDirectSoundBuffer_Lock(globalDSSecondaryBuffer, ByteToLock, BytesToWrite,
                                                            &R01, &R01_size, &R02, &R02_size, 0))) {
        int16_t *SampleOut;
        DWORD SampleIndex;

        SampleOut = (int16_t *)R01;
        DWORD R01_sample_count = R01_size / soundout->BytesPerSample;
        for (SampleIndex = 0; SampleIndex < R01_sample_count; ++SampleIndex) {
            float t = 2.0l * PI * ((float)(soundout->running_sample_index) / (float)(soundout->square_wave_period));
            float sineval = sinf(t);
            int16_t sample_val = (int16_t)(sineval * soundout->tone_volume);
            *SampleOut++ = sample_val;
            *SampleOut++ = sample_val;
            ++soundout->running_sample_index;
        }

        SampleOut = (int16_t *)R02;
        DWORD R02_sample_count = R02_size / soundout->BytesPerSample;
        for (SampleIndex = 0; SampleIndex < R02_sample_count; ++SampleIndex) {
            float t = 2.0l * PI * ((float)(soundout->running_sample_index) / (float)(soundout->square_wave_period));
            float sineval = sinf(t);
                int16_t sample_val = (int16_t)(sineval * soundout->tone_volume);
                *SampleOut++ = sample_val;
                *SampleOut++ = sample_val;
                ++soundout->running_sample_index;
            }
            IDirectSoundBuffer_Unlock(globalDSSecondaryBuffer, R01, R01_size, R02, R02_size);
        }
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSA WindowClass;

    memset(&WindowClass, 0, sizeof(WNDCLASSA));

    // TODO: our best size is 1009 to 486 instead of 1280 to 720
    win32_ResizeDIBsect(&globalbackbuffer, 1009, 486);

    WindowClass.style = CS_HREDRAW|CS_VREDRAW;
    WindowClass.lpfnWndProc = WindowProc;
    WindowClass.cbClsExtra = 0;
    WindowClass.cbWndExtra = 0;
    WindowClass.hInstance = 0;
    WindowClass.hIcon = 0;
    WindowClass.hCursor = 0;
    WindowClass.hbrBackground = 0;
    WindowClass.lpszMenuName = 0;
    WindowClass.lpszClassName = "HandmadeWindowClass";

    if(RegisterClassA(&WindowClass)) {
        HWND Window = CreateWindowExA(0, WindowClass.lpszClassName, "Handmade", WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                                      CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, hInstance, 0);

        if (Window) {
            LARGE_INTEGER starting_time;
            LARGE_INTEGER frequency;
            uint64_t cycle_start = __rdtsc();

            int xoffset = 0;
            int yoffset = 0;

            struct win32_sound_output soundout;
            memset(&soundout, 0, sizeof(struct win32_sound_output));
            soundout.SamplesPerSecond = 44100;
            soundout.ToneHz = 440;
            soundout.square_wave_period = soundout.SamplesPerSecond / soundout.ToneHz;
            soundout.BytesPerSample = sizeof(int16_t) * 2;
            soundout.SecondaryBufferSize = soundout.SamplesPerSecond * soundout.BytesPerSample;
            soundout.tone_volume = 3000;
            soundout.running_sample_index = 0;
            win32_init_direct_sound(Window, soundout.SamplesPerSecond, soundout.SecondaryBufferSize);
            win32_fill_sound_buffer(&soundout, 0, soundout.SecondaryBufferSize);
            IDirectSoundBuffer_Play(globalDSSecondaryBuffer, 0, 0, DSBPLAY_LOOPING);

            global_running = 1;

            QueryPerformanceFrequency(&frequency);
            QueryPerformanceCounter(&starting_time);

            while (global_running) {
                MSG Message;
                while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE)) {
                    if (Message.message == WM_QUIT) {
                        global_running = 0;
                    }
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }

                // Platform independent!!
                struct game_offscreen_buffer game_buffer;
                memset(&game_buffer, 0, sizeof(struct game_offscreen_buffer));
                game_buffer.memory = globalbackbuffer.memory;
                game_buffer.width = globalbackbuffer.width;
                game_buffer.height = globalbackbuffer.height;
                game_buffer.pitch = globalbackbuffer.pitch;
                game_update_and_render(&game_buffer, xoffset, yoffset);
                // Platform independent!!

                // sound!
                DWORD PlayCursor;
                DWORD WriteCursor;
                if (SUCCEEDED(IDirectSoundBuffer_GetCurrentPosition(globalDSSecondaryBuffer, &PlayCursor, &WriteCursor))) {

                    DWORD ByteToLock = (soundout.running_sample_index*soundout.BytesPerSample) % soundout.SecondaryBufferSize;
                    DWORD BytesToWrite;

                    if (ByteToLock > PlayCursor) {
                        BytesToWrite = (soundout.SecondaryBufferSize - ByteToLock);
                        BytesToWrite += PlayCursor;
                    } else {
                        BytesToWrite = PlayCursor - ByteToLock;
                    }
                    win32_fill_sound_buffer(&soundout, ByteToLock, BytesToWrite);
                }
                // sound!

                HDC DeviceContext = GetDC(Window);
                struct win32_window_dimension dim = win32_get_window_dimension(Window);
                win32_Update(DeviceContext, &globalbackbuffer, dim.width, dim.height);
                ReleaseDC(Window, DeviceContext);

                if (global_movement == VK_LEFT) xoffset -= 8;
                if (global_movement == VK_RIGHT) xoffset += 8;
                if (global_movement == VK_UP) yoffset -= 8;
                if (global_movement == VK_DOWN) yoffset += 8;

                uint64_t cycle_end = __rdtsc();

                LARGE_INTEGER ending_time;
                QueryPerformanceCounter(&ending_time);

                // TODO: test with -O2
                // TODO: change to floating point numbers

                uint64_t ellapsed_cycles = cycle_end - cycle_start;
                uint64_t ellapsed_microseconds = ending_time.QuadPart - starting_time.QuadPart;
                // microseconds * 1000 => milliseconds / frequency in Hz (MHz)
                uint32_t mills_per_frame = ((1000*ellapsed_microseconds) / frequency.QuadPart);
                uint32_t fps = frequency.QuadPart / ellapsed_microseconds;
                uint32_t mhz_per_frame = (uint32_t)ellapsed_cycles / (1000*1000);

                //printf(" %d mills/frame     %d frame/second     %d mhz/frame\n", mills_per_frame, fps, mhz_per_frame);

                starting_time = ending_time;
                cycle_start = cycle_end;
            }
        } else {
            MessageBox(NULL, "Window does not exist!", "ERROR", MB_OK);
        }
    } else {
        MessageBox(NULL, "WNDCLASS registration failed!", "ERROR", MB_OK);
    }
    return 0;
}
