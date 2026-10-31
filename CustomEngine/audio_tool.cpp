#include "raylib.h"
#include <stdio.h>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: audio_tool <input_file> <output_wav>\n");
        return 1;
    }
    InitAudioDevice();
    Wave w = LoadWave(argv[1]);
    if (w.frameCount == 0) {
        printf("Failed to load: %s\n", argv[1]);
        CloseAudioDevice();
        return 2;
    }
    WaveFormat(&w, 44100, 16, 1);
    bool ok = ExportWave(w, argv[2]);
    printf("Exported: %s (ok=%d, frames=%u)\n", argv[2], ok, w.frameCount);
    UnloadWave(w);
    CloseAudioDevice();
    return ok ? 0 : 3;
}

