/*
 * Copyright (C) 2020-2024, HENSOLDT Cyber GmbH
 * 
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * For commercial licensing, contact: info.cyber@hensoldt.net
 */

#include "OS_Error.h"

#include "lib_utils/RleCompressor.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>

static void
die(
    const char* fmt,
    ...)
{
    va_list vl;

    /*
     * Output fatal error message and exit
     */

    printf("FATAL: ");

    va_start(vl, fmt);
    vprintf(fmt, vl);
    va_end(vl);

    printf("\nExiting.\n");

    exit(-1);
}

static size_t
read_file(
    const char* fname,
    uint8_t** buf)
{
    size_t sz;
    FILE* f;

    /*
     * Read the entire file into a newly allocated buffer
     */

    f = fopen(fname, "rb");
    fseek(f, 0, SEEK_END);
    sz = ftell(f);
    fseek(f, 0, SEEK_SET);

    if ((*buf = malloc(sz)) == NULL)
    {
        die("Failed to allocate buffer of %zu bytes for file contents.", sz);
    }
    if (fread(*buf, 1, sz, f) != sz)
    {
        die("Failed to read expected amount of bytes from file.");
    }

    fclose(f);

    return sz;
}

static size_t
write_file(
    const char* fname,
    const size_t sz_org,
    const double perc,
    const size_t sz,
    const uint8_t* buf)
{
    size_t wr, i;
    FILE* fp;

    /*
     * Write out into C array with the symbols the RamDisk expects..
     */

    fp = fopen(fname, "w");
    fprintf(fp, "#include <stdint.h>\n");
    fprintf(fp, "#include <stddef.h>\n");
    fprintf(fp, "//\n");
    fprintf(fp, "// Generated with rdgen\n");
    fprintf(fp, "// Original was %zu bytes, now just %zu bytes (%.4f%%)\n",
            sz_org, sz, perc);
    fprintf(fp, "//\n");
    fprintf(fp, "uint8_t RAMDISK_IMAGE[] = {");
    for (i = 0; i < sz; i++)
    {
        if (i % 15 == 0)
        {
            fprintf(fp, "\n    ");
        }
        fprintf(fp, "0x%02x,", buf[i]);
    }
    fprintf(fp, "\n};\n");
    fprintf(fp, "size_t RAMDISK_IMAGE_SIZE = sizeof(RAMDISK_IMAGE);\n");

    wr = ftell(fp);
    fclose(fp);

    return wr;
}

int
main(
    int argc,
    char* argv[])
{
    OS_Error_t err;
    uint8_t* org_buf, *buf;
    size_t sz, sz_org, sz_buf;
    double perc;

    printf("rdgen: Compress NVM image into RLE encoded RamDisk format\n\n");
    if (argc != 3)
    {
        printf("Usage: %s <nvm> <img>\n", argv[0]);
        return 0;
    }

    // Make sure file actually exists
    if (access(argv[1], F_OK) == -1)
    {
        die("File '%s' cannot be accessed. Does it exist?", argv[1]);
    }

    // Read whole file into allocated buffer;
    sz_org = read_file(argv[1], &org_buf);

    // Do the compression, let it alloc the buffer for us
    if ((err = RleCompressor_compress(sz_org, org_buf, 0, &sz,
                                      &buf)) != OS_SUCCESS)
    {
        die("RleCompressor_compress() failed with %i", err);
    }

    // Print sizes and compression ratio
    perc = (float)sz / (float)sz_org * 100.0;
    printf("Original size:   %12zu bytes\n", sz_org);
    printf("Compressed size: %12zu bytes (%.4f%%)\n", sz, perc);

    // Write to output file
    sz_buf = write_file(argv[2], sz_org, perc, sz, buf);

    // Decompress again
    if ((err = RleCompressor_decompress(sz, buf, sz_org, &sz,
                                        &org_buf)) != OS_SUCCESS)
    {
        die("RleCompressor_decompress() failed with %i", err);
    }

    // Re-read file and check it matches
    free(buf);
    sz_buf = read_file(argv[1], &buf);
    if (sz_buf != sz)
    {
        die("Decompression result differs in size.");
    }
    if (memcmp(org_buf, buf, sz))
    {
        die("Decompression result is does not match original file.");
    }

    free(buf);
    free(org_buf);

    return 0;
}