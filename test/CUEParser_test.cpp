#include "CUEParser.h"
#include <stdio.h>
#include <string.h>

/* Unit test helpers */
#define COMMENT(x) printf("\n----" x "----\n");
#define TEST(x) \
    if (!(x)) { \
        fprintf(stderr, "\033[31;1mFAILED:\033[22;39m %s:%d %s\n", __FILE__, __LINE__, #x); \
        status = false; \
    } else { \
        printf("\033[32;1mOK:\033[22;39m %s\n", #x); \
    }

bool test_basics()
{
    bool status = true;
    const char *cue_sheet = R"(
FILE "Image Name.bin" BINARY
  TRACK 01 MODE1/2048
    INDEX 01 00:00:00
  TRACK 02 AUDIO
    PREGAP 00:02:00
    INDEX 01 02:47:20
  TRACK 03 AUDIO
    INDEX 00 07:55:58
    INDEX 01 07:55:65
FILE "Sound.wav" WAVE
  TRACK 11 AUDIO
    INDEX 00 00:00:00
    INDEX 01 00:02:00
    )";

    CUEParser parser(cue_sheet);

    COMMENT("test_basics()");
    COMMENT("Test TRACK 01 (data)");
    const CUETrackInfo *track = parser.next_track();
    TEST(track != NULL);
    if (track)
    {
        TEST(strcmp(track->filename, "Image Name.bin") == 0);
        TEST(track->file_mode == CUEFile_BINARY);
        TEST(track->file_offset == 0);
        TEST(track->track_number == 1);
        TEST(track->track_mode == CUETrack_MODE1_2048);
        TEST(track->sector_length == 2048);
        TEST(track->unstored_pregap_length == 0);
        TEST(track->data_start == 0);
    }

    COMMENT("Test TRACK 02 (audio with pregap)");
    track = parser.next_track();
    TEST(track != NULL);
    uint32_t start2 = ((2 * 60) + 47) * 75 + 20;
    uint32_t pregap_offset = 2 * 75;
    if (track)
    {
        TEST(strcmp(track->filename, "Image Name.bin") == 0);
        TEST(track->file_mode == CUEFile_BINARY);
        TEST(track->file_offset == 2048 * start2);
        TEST(track->track_number == 2);
        TEST(track->track_mode == CUETrack_AUDIO);
        TEST(track->sector_length == 2352);
        TEST(track->unstored_pregap_length == pregap_offset);
        TEST(track->data_start == start2 + pregap_offset);
    }

    COMMENT("Test TRACK 03 (audio with index 0)");
    track = parser.next_track();
    TEST(track != NULL);
    uint32_t start3_i0 = ((7 * 60) + 55) * 75 + 58;
    uint32_t start3_i1 = ((7 * 60) + 55) * 75 + 65;
    if (track)
    {
        TEST(strcmp(track->filename, "Image Name.bin") == 0);
        TEST(track->file_mode == CUEFile_BINARY);
        TEST(track->file_offset == 2048 * start2 + 2352 * (start3_i1 - start2));
        TEST(track->track_number == 3);
        TEST(track->track_mode == CUETrack_AUDIO);
        TEST(track->sector_length == 2352);
        TEST(track->track_start == start3_i0 + pregap_offset);
        TEST(track->data_start == start3_i1 + pregap_offset);
    }

    COMMENT("Test TRACK 11 (audio from wav)");
    uint32_t track03_lba_length = 4 * 75;
    uint32_t prev_data_start = track->data_start;
    // Because the FILE restarts MSF locations we need the lba offset it starts at
    uint32_t zeroed_lba_offset = prev_data_start + track03_lba_length;
    track = parser.next_track(track->file_offset + track03_lba_length * 2352);
    TEST(track != NULL);
    uint32_t start11_i0 = zeroed_lba_offset + 0;
    uint32_t start11_i1 = zeroed_lba_offset + (2 * 75);
    if (track)
    {
        TEST(strcmp(track->filename, "Sound.wav") == 0);
        TEST(track->file_mode == CUEFile_WAVE);
        TEST(track->file_offset == 0);
        TEST(track->track_number == 11);
        TEST(track->track_mode == CUETrack_AUDIO);
        TEST(track->sector_length == 0);
        TEST(track->track_start == start11_i0 + pregap_offset);
        TEST(track->data_start == start11_i1 + pregap_offset);
    }

    COMMENT("Test end of file");
    track = parser.next_track();
    TEST(track == NULL);

    COMMENT("Test restart");
    parser.restart();
    track = parser.next_track();
    TEST(track != NULL && track->track_number == 1);

    return status;
}

bool test_datatracks()
{
    bool status = true;
    const char *cue_sheet = R"(
FILE "beos-5.0.3-professional-gobe.bin" BINARY
TRACK 01 MODE1/2352
    INDEX 01 00:00:00
TRACK 02 MODE1/2352
    INDEX 01 10:48:58
TRACK 03 MODE1/2352
    INDEX 01 46:07:03
    )";

    CUEParser parser(cue_sheet);

    COMMENT("test_datatracks()");
    COMMENT("Test TRACK 01 (data)");
    const CUETrackInfo *track = parser.next_track();
    TEST(track != NULL);
    if (track)
    {
        TEST(strcmp(track->filename, "beos-5.0.3-professional-gobe.bin") == 0);
        TEST(track->file_mode == CUEFile_BINARY);
        TEST(track->file_offset == 0);
        TEST(track->track_number == 1);
        TEST(track->track_mode == CUETrack_MODE1_2352);
        TEST(track->sector_length == 2352);
        TEST(track->unstored_pregap_length == 0);
        TEST(track->data_start == 0);
        TEST(track->track_start == 0);
    }

    COMMENT("Test TRACK 02 (data)");
    track = parser.next_track();
    TEST(track != NULL);
    if (track)
    {
        TEST(track->file_mode == CUEFile_BINARY);
        TEST(track->file_offset == 0x6D24560);
        TEST(track->track_number == 2);
        TEST(track->track_mode == CUETrack_MODE1_2352);
        TEST(track->sector_length == 2352);
        TEST(track->unstored_pregap_length == 0);
        TEST(track->data_start == ((10 * 60) + 48) * 75 + 58);
        TEST(track->track_start == ((10 * 60) + 48) * 75 + 58);
    }

    COMMENT("Test TRACK 03 (data)");
    track = parser.next_track();
    TEST(track != NULL);
    if (track)
    {
        TEST(track->file_mode == CUEFile_BINARY);
        TEST(track->file_offset == 0x1D17E780);
        TEST(track->track_number == 3);
        TEST(track->track_mode == CUETrack_MODE1_2352);
        TEST(track->sector_length == 2352);
        TEST(track->unstored_pregap_length == 0);
        TEST(track->data_start == ((46 * 60) + 7) * 75 + 3);
        TEST(track->track_start == ((46 * 60) + 7) * 75 + 3);
    }

    track = parser.next_track();
    TEST(track == NULL);

    return status;
}

bool test_datatrackpregap()
{
    bool status = true;
    const char *cue_sheet = R"(
FILE "issue422.bin" BINARY
  TRACK 01 AUDIO
    INDEX 01 00:00:00
  TRACK 02 MODE1/2352
    PREGAP 00:02:00
    INDEX 01 01:06:19
    )";

    CUEParser parser(cue_sheet);

    COMMENT("test_datatrackpregap()");
    COMMENT("Test TRACK 01 (audio)");
    const CUETrackInfo *track = parser.next_track();
    TEST(track != NULL);
    if (track)
    {
        TEST(strcmp(track->filename, "issue422.bin") == 0);
        TEST(track->file_mode == CUEFile_BINARY);
        TEST(track->file_offset == 0);
        TEST(track->file_index == 1);
        TEST(track->track_number == 1);
        TEST(track->track_mode == CUETrack_AUDIO);
        TEST(track->sector_length == 2352);
        TEST(track->unstored_pregap_length == 0);
        TEST(track->data_start == 0);
        TEST(track->track_start == 0);
    }

    COMMENT("Test TRACK 02 (data)");
    track = parser.next_track();
    TEST(track != NULL);
    if (track)
    {
        TEST(strcmp(track->filename, "issue422.bin") == 0);
        TEST(track->file_mode == CUEFile_BINARY);
        TEST(track->file_offset == 0xB254B0);
        TEST(track->file_index == 1);
        TEST(track->track_number == 2);
        TEST(track->track_mode == CUETrack_MODE1_2352);
        TEST(track->sector_length == 2352);
        TEST(track->unstored_pregap_length == 75 * 2);
        TEST(track->data_start == (60 + 6 + 2) * 75 + 19);
        TEST(track->track_start == (60 + 6) * 75 + 19);
    }

    track = parser.next_track();
    TEST(track == NULL);

    return status;
}

bool test_multifile()
{
    bool status = true;
    const char *cue_sheet = R"(
CATALOG 0000000000000
FILE "track1.bin" BINARY
  TRACK 01 MODE1/2352
    INDEX 01 00:00:00
FILE "track2.bin" BINARY
  TRACK 02 MODE1/2352
    INDEX 00 00:00:00
    INDEX 01 00:02:01
    )";

    CUEParser parser(cue_sheet);

    COMMENT("test_multifile()");
    COMMENT("Test TRACK 01 (data)");
    const CUETrackInfo *track = parser.next_track(0);
    TEST(track != NULL);
    if (track)
    {
        TEST(strcmp(track->filename, "track1.bin") == 0);
        TEST(track->file_mode == CUEFile_BINARY);
        TEST(track->file_offset == 0);
        TEST(track->file_index == 1);
        TEST(track->track_number == 1);
        TEST(track->track_mode == CUETrack_MODE1_2352);
        TEST(track->sector_length == 2352);
        TEST(track->unstored_pregap_length == 0);
        TEST(track->data_start == 0);
        TEST(track->track_start == 0);
    }

    COMMENT("Test TRACK 02 (data)");
    track = parser.next_track(2352 * 301); // track1.bin size 707952 bytes
    TEST(track != NULL);
    if (track)
    {
        TEST(strcmp(track->filename, "track2.bin") == 0);
        TEST(track->file_mode == CUEFile_BINARY);
        TEST(track->file_offset == (2 * 75 + 1) * 2352);
        TEST(track->file_index == 2);
        TEST(track->track_number == 2);
        TEST(track->track_mode == CUETrack_MODE1_2352);
        TEST(track->sector_length == 2352);
        TEST(track->unstored_pregap_length == 0);
        TEST(track->data_start == (4 + 2) * 75 + 1 + 1);
        TEST(track->track_start == 4 * 75 + 1);
    }

    track = parser.next_track(14594160);
    TEST(track == NULL);

    return status;
}

bool test_dot_slash_removal()
{
    bool status = true;
    const char *cue_sheet = R"(
CATALOG 0000000000000
FILE "./track1.bin" BINARY
  TRACK 01 MODE1/2352
    INDEX 01 00:00:00
FILE ".\track2.bin" BINARY
  TRACK 02 MODE1/2352
    INDEX 00 00:00:00
    INDEX 01 00:02:01
    )";

    CUEParser parser(cue_sheet);
    COMMENT("test_dot_slash_removal()");
    COMMENT("Test TRACK 01 (data)");
    const CUETrackInfo *track = parser.next_track(0);
    TEST(track != NULL);
    if (track)
    {
        TEST(strcmp(track->filename, "track1.bin") == 0);
        TEST(track->file_mode == CUEFile_BINARY);
        TEST(track->file_offset == 0);
        TEST(track->file_index == 1);
        TEST(track->track_number == 1);
        TEST(track->track_mode == CUETrack_MODE1_2352);
        TEST(track->sector_length == 2352);
        TEST(track->unstored_pregap_length == 0);
        TEST(track->data_start == 0);
        TEST(track->track_start == 0);
    }

    COMMENT("Test TRACK 02 (data)");
    track = parser.next_track(2352 * 301); // track1.bin size 707952 bytes
    TEST(track != NULL);
    if (track)
    {
        TEST(strcmp(track->filename, "track2.bin") == 0);
        TEST(track->file_mode == CUEFile_BINARY);
        TEST(track->file_offset == (2 * 75 + 1) * 2352);
        TEST(track->file_index == 2);
        TEST(track->track_number == 2);
        TEST(track->track_mode == CUETrack_MODE1_2352);
        TEST(track->sector_length == 2352);
        TEST(track->unstored_pregap_length == 0);
        TEST(track->data_start == (4 + 2) * 75 + 1 + 1);
        TEST(track->track_start == 4 * 75 + 1);
    }
    return status;
}

bool test_long_filename()
{
    bool status = true;
    const char *cue_sheet = R"(
CATALOG 0000000000000
FILE "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000.bin" BINARY
  TRACK 01 MODE1/2352
    INDEX 01 00:00:00
    )";

    CUEParser parser(cue_sheet);
    COMMENT("test_long_filename()");
    COMMENT("Test TRACK 01 (data)");
    const CUETrackInfo *track = parser.next_track(0);
    TEST(track != NULL);
    if (track)
    {
        TEST(strcmp(track->filename, "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000.bin") == 0);
        TEST(track->file_mode == CUEFile_BINARY);
        TEST(track->file_offset == 0);
        TEST(track->file_index == 1);
        TEST(track->track_number == 1);
        TEST(track->track_mode == CUETrack_MODE1_2352);
        TEST(track->sector_length == 2352);
        TEST(track->unstored_pregap_length == 0);
        TEST(track->data_start == 0);
        TEST(track->track_start == 0);
    }
    return status;
}

int main()
{
    if (test_basics() && test_datatracks() && test_datatrackpregap() && test_multifile() && test_dot_slash_removal() && test_long_filename())
    {
        return 0;
    }
    else
    {
        printf("Some tests failed\n");
        return 1;
    }
}
