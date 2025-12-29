#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>

/* Pipes:
 * https://tldp.org/LDP/lpg/node12.html
 * https://stackoverflow.com/questions/31906192/how-to-use-environment-variable-in-a-c-program
 */

/*
 * TODO: handle endianess
 * TODO: handle interger sizes correctly.
 * 
 * NDS: is little endian
 *      long long int == int64_t
 *      long          == int32_t
 *      int           == int32_t
 *      short         == int16_t
 *      char          == int8_t | uint8_t
 */

#define ARM9_FIRMWARE_ADDRESS 0x02000000

#define ARM9_FIRMWARE_SECTION_ARRAY_REF 0x02000b4c
#define ARM9_FIRMWARE_SECTION_ARRAY_LOC 0x020a5a00
#define ARM9_FIRMWARE_SECTION_DATA 0x020a5380

#define ITCM_SECTION_DATA_LOC 0x020a5380
#define ITCM_START 0x01ff8000
#define ITCM_SIZE 0x620
#define ITCM_BSS 0

#define DTCM_SECTION_DATA_LOC 0x20A59A0
#define DTCM_START 0x027c0000
#define DTCM_SIZE 0x60
#define DTCM_BSS 0

const char *devkit_envvar = "DEVKITARM";

typedef struct buffer buffer; 

struct buffer {
  int length;
  char *data;
};

int str_suffix(const char *str, const char *suffix);

buffer *load_binary_file(const char *path);

int save_binary_file(const char *path, buffer *data);

int buffer_write(buffer *b, int offset, buffer *data);

int buffer_write_bytes(buffer *b, int offset, int count, void *data);

buffer *buffer_get_slice(buffer *src, int offset, int count);

void destroy_section_info(char **info);

void destroy_buffer(buffer *b)
{
  if (b == NULL) {
    return;
  }

  free(b->data);
  free(b);
}

buffer *buffer_get_slice(buffer *src, int offset, int count)
{
  if (count < 1 || offset < 0 || offset + count > src->length) {
    return NULL;
  }

  buffer *s = malloc(sizeof *s);

  if (s == NULL) {
    return NULL;
  }

  *s = (buffer) {
    .data = malloc(count),
    .length = count
  };

  if (s->data == NULL) {
    free(s);
    return NULL;
  }

  memcpy(s->data, &src->data[offset], count);

  return s;
}

int buffer_write(buffer *b, int offset, buffer *data)
{
  if (offset > b->length || offset < 0) {
    return 0;
  }

  if (b->length < offset + data->length) {
    int new_length = offset + data->length;
    char *new_data = realloc(b->data, new_length);

    if (new_data == NULL) {
      return 0;
    }

    b->data = new_data;
    b->length = new_length;
  }

  memcpy(&b->data[offset], data->data, data->length);

  return 1;
}

int buffer_write_bytes(buffer *b, int offset, int count, void *data)
{
  if (offset > b->length || offset < 0) {
    return 0;
  }

  if (b->length < offset + count) {
    int new_length = offset + count;
    char *new_data = realloc(b->data, new_length);

    if (new_data == NULL) {
      return 0;
    }

    b->data = new_data;
    b->length = new_length;
  }

  memcpy(&b->data[offset], data, count);

  return 1;
}


int str_suffix(const char *str, const char *suffix)
{
  int str_len = strlen(str);
  int suffix_len = strlen(suffix);

  if (str_len < suffix_len) {
    return 0;
  }

  const char *str_ptr = str + str_len - suffix_len;

  return strcmp(str_ptr, suffix) == 0;
}

void destroy_section_info(char **info)
{
  if (info == NULL) {
    return;
  }

  for(int i = 0; info[i] != NULL; i++) {
    free(info[i]);
  }

  free(info);
}

buffer *load_binary_file(const char *path)
{
  FILE *f = fopen(path, "rb");

  if (f == NULL) {
    return NULL;
  }

  fseek(f, 0, SEEK_END);
  int size = ftell(f);
  fseek(f, 0, SEEK_SET);

  void *data = malloc(size);

  if (data == NULL) {
    fclose(f);
    return NULL;
  }

  int read = fread(data, 1, size, f);

  fclose(f);

  if (read != size) {
    free(data);
    return NULL;
  }

  buffer *b = malloc(sizeof *b);

  if (b == NULL) {
    free(data);
    return NULL;
  }

  *b = (buffer) {
      .data = data,
      .length = size
  };

  return b;
}

int save_binary_file(const char *path, buffer *data)
{
  FILE *f = fopen(path, "wb");

  if (f == NULL) {
    return 0;
  }

  if (fwrite(data->data, 1, data->length, f) != data->length) {
    fclose(f);
    return 0;
  }

  return fclose(f) == 0 ? 1 : 0;
}

buffer *extract_section_data(const char *object_file, const char *section)
{
  const char *devkit = getenv(devkit_envvar);

  if (devkit == NULL) {
    return NULL;
  }

  char tmp[L_tmpnam];

  if (tmpnam(tmp) == NULL) {
    return NULL;
  }

  char cmd[1024];
  int s = snprintf(cmd, sizeof cmd,
                   "%s/bin/arm-none-eabi-objcopy -O binary -j %s %s %s", devkit,
                   section, object_file, tmp);
  
  if (s < 0 || s >= sizeof cmd) {
    return NULL;
  }

  int code = system(cmd);

  if (code != 0) {
    return NULL;
  }

  buffer *b = load_binary_file(tmp);

  remove(tmp);

  return b;
}

char **get_section_header_info(char *object_file)
{
  char *devkit = getenv(devkit_envvar);

  if (devkit == NULL) {
    return NULL;
  }

  char cmd[1024];
  int s = snprintf(cmd, sizeof cmd, "%s/bin/arm-none-eabi-objdump -h %s",
                   devkit, object_file);

  if (s < 0 || s >= sizeof cmd) {
    return NULL;
  }

  FILE *in = popen(cmd, "r");

  if (in == NULL) {
    printf("Failed to execute command: %s\n", cmd);
    return NULL;
  }

  char line[1024];
  int capacity = 50;
  int count = 0;

  char **lines = malloc(50 * sizeof(char *));

  if (lines == NULL) {
    pclose(in);
    return NULL;
  }

  lines[0] = NULL;

  while (fgets(line, sizeof line, in) != NULL) {
    if (count == capacity - 1) {
      int new_capacity = capacity + 50;
      char **new_lines = realloc(lines, new_capacity);

      if (new_lines == NULL) {
        break;
      }

      capacity = new_capacity;
      lines = new_lines;
    }

    char *str = strdup(line);

    if (str == NULL) {
      break;
    }

    lines[count++] = str;
    lines[count] = NULL;
  }

  if (ferror(in)) {
    destroy_section_info(lines);
    lines = NULL;
  } else if (!feof(in)) {
    destroy_section_info(lines);
    lines = NULL;
  }

  int code = pclose(in);

  if (code != 0) {
    destroy_section_info(lines);
    lines = NULL;
  }

  return lines;
}

int apply_patches(char *firmware_file, char *object_file)
{
  buffer *fw = load_binary_file(firmware_file);

  if (fw == NULL) {
    return 0;
  }

  int result = 1;
  char name[256];
  int size;
  int vma;
  char **section_info = get_section_header_info(object_file);

  if (section_info == NULL) {
    result = 0;
    goto error;
  }

  for (int i = 0; section_info[i] != NULL; i++) {
    if (sscanf(section_info[i], "%*d %255s %x %x", name, &size, &vma) != 3) {
      continue;
    }

    if (!str_suffix(name, "_patch")) {
      continue;
    }

    buffer *b = extract_section_data(object_file, name);

    if (b == NULL) {
      result = 0;
      break;
    }

    printf("writing patch: %s of size %d at 0x%x\n", name, size, vma);
    int offset = vma - ARM9_FIRMWARE_ADDRESS;
    result = buffer_write(fw, offset, b);
    destroy_buffer(b);

    if (result == 0) {
      break;
    }
  }

  if (result == 1) {
    result = save_binary_file(firmware_file, fw);
  }

error:
  destroy_buffer(fw);
  destroy_section_info(section_info);

  return result;
}

int apply_tcm_patches(char *firmware_file, char *object_file)
{
  buffer *itcm_data = NULL;
  buffer *dtcm_data = NULL;
  buffer *fw_itcm_data = NULL;
  buffer *fw_dtcm_data = NULL;
  buffer *fw_footer_data = NULL;
  buffer *fw = load_binary_file(firmware_file);

  if (fw == NULL) {
    return 0;
  }

  int tcm_bss_size = 0;
  int result = 0;
  char name[256];
  int size;
  int vma;
  char **section_info = get_section_header_info(object_file);

  if (section_info == NULL) {
    goto error;
  }

  for (int i = 0; section_info[i] != NULL; i++) {
    if (sscanf(section_info[i], "%*d %255s %x %x", name, &size, &vma) != 3) {
      continue;
    }

    if (strcmp(name, "text_tcm_extend") == 0) {
      itcm_data = extract_section_data(object_file, "text_tcm_extend");

      if (itcm_data == NULL) {
        goto error;
      }
      printf("found ITCM extension of size: %d\n", itcm_data->length);
    } 

    if (strcmp(name, "data_tcm_extend") == 0) {
      dtcm_data = extract_section_data(object_file, "data_tcm_extend");

      if (dtcm_data == NULL) {
        goto error;
      }
      printf("found DTCM extension of size: %d\n", dtcm_data->length);
    }

    if (strcmp(name, "bss_tcm_extend") == 0) {
      tcm_bss_size = size;
      printf("found DTCM BSS extension if size: %d\n", size);
    }
  }

  int itcm_offset = ITCM_SECTION_DATA_LOC - ARM9_FIRMWARE_ADDRESS;
  fw_itcm_data = buffer_get_slice(fw, itcm_offset, ITCM_SIZE);

  if (fw_itcm_data == NULL) {
    goto error;
  }

  if (itcm_data != NULL) {
    if (buffer_write(fw_itcm_data, fw_itcm_data->length, itcm_data) == 0) {
      goto error;
    }
  }

  int dtcm_offset = DTCM_SECTION_DATA_LOC - ARM9_FIRMWARE_ADDRESS;
  fw_dtcm_data = buffer_get_slice(fw, dtcm_offset, DTCM_SIZE);

  if (fw_dtcm_data == NULL) {
    goto error;
  }

  if (dtcm_data != NULL) {
    if (buffer_write(fw_dtcm_data, fw_dtcm_data->length, dtcm_data) == 0) {
      goto error;
    }
  }

  fw_footer_data = buffer_get_slice(fw, fw->length - 12, 12);

  if (fw_footer_data == NULL) {
    goto error;
  }

  int section_data[6] = {
    0x01ff8000, fw_itcm_data->length, 0,
    0x027c0000, fw_dtcm_data->length, tcm_bss_size
  };

  int fw_itcm_data_offset = ARM9_FIRMWARE_SECTION_DATA - ARM9_FIRMWARE_ADDRESS;
  int fw_dtcm_data_offset = fw_itcm_data_offset + fw_itcm_data->length;
  int secarr_start_offset = fw_dtcm_data_offset + fw_dtcm_data->length;
  int secarr_end_offset = secarr_start_offset + sizeof section_data;

  if (buffer_write(fw, fw_itcm_data_offset, fw_itcm_data) == 0) {
    goto error;
  }

  if (buffer_write(fw, fw_dtcm_data_offset, fw_dtcm_data) == 0) {
    goto error;
  }

  if (buffer_write_bytes(fw, secarr_start_offset, sizeof section_data, section_data) == 0) {
    goto error;
  }

  if (buffer_write(fw, secarr_end_offset, fw_footer_data) == 0) {
    goto error;
  }

  int crt0_ref_offset = ARM9_FIRMWARE_SECTION_ARRAY_REF - ARM9_FIRMWARE_ADDRESS;
  int secarr_start_address = secarr_start_offset + ARM9_FIRMWARE_ADDRESS;
  int secarr_end_address = secarr_end_offset + ARM9_FIRMWARE_ADDRESS;

  buffer_write_bytes(fw, crt0_ref_offset, 4, &secarr_start_address);
  buffer_write_bytes(fw, crt0_ref_offset + 4, 4, &secarr_end_address);

  result = save_binary_file(firmware_file, fw);

error:
  destroy_buffer(fw);
  destroy_buffer(itcm_data);
  destroy_buffer(dtcm_data);
  destroy_buffer(fw_itcm_data);
  destroy_buffer(fw_dtcm_data);
  destroy_buffer(fw_footer_data);
  destroy_section_info(section_info);

  return result;
}

int main(int argc, char **argv)
{
  if (argc != 3) {
    printf("Usage: patch_tool <object_file> <firmware binary>\n");
    return 1;
  }

  char *object_file = argv[1];
  char *firmware_file = argv[2];

  if (apply_patches(firmware_file, object_file) == 0) {
    return 1;
  }

  if (apply_tcm_patches(firmware_file, object_file) == 0) {
    return 1;
  }

  return 0;
}
