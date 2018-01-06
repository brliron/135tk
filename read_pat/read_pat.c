#include <assert.h>
#include <string.h>
#include "read_pat.h"

#define READ_DEF(n)						\
uint##n##_t  read_u##n(FILE* fp, json_t *js, const char *name)	\
{								\
  uint##n##_t num;						\
  json_t *rep = json_object_get(js, name);			\
  if (js && rep && json_is_integer(rep)) {			\
    num = json_integer_value(rep);				\
    fwrite(&num, n/8, 1, fp);					\
  }								\
  else {							\
    assert(fread(&num, 1, n/8, fp) == n/8);			\
    if (js) json_object_set_new(js, name, json_integer(num));	\
  }								\
  								\
  return num;							\
}

READ_DEF(8)
READ_DEF(16)
READ_DEF(32)
READ_DEF(64)

float	read_float(FILE* fp, json_t *js, const char *name)
{
  float	num;

  json_t *rep = json_object_get(js, name);
  if (js && rep && json_is_real(rep)) {
    num = json_real_value(rep);
    fwrite(&num, 4, 1, fp);
  }
  else {
    assert(fread(&num, 1, 4, fp) == 4);
    if (js) json_object_set_new(js, name, json_real(num));
  }

  return num;
}

void		read_str(FILE* fp, char* buffer, size_t size, json_t *js, const char *name)
{
  json_t	*rep = json_object_get(js, name);
  if (js && rep && json_is_string(rep))
    {
      memcpy(buffer, json_string_value(rep), size);
      fwrite(buffer, size, 1, fp);
    }
  else
    {
      assert(fread(buffer, 1, size, fp) == size);
      if (js) json_object_set_new(js, name, json_stringn(buffer, size));
    }
}

void		read_bytes(FILE* fp, void* buffer_, size_t size, json_t *js, const char *name)
{
  char*		buffer = (char*)buffer_;
  json_t	*rep = json_object_get(js, name);
  if (js && rep && json_is_string(rep))
    {
      assert(size * 2 == json_string_length(rep));
      const char *rep_s = json_string_value(rep);
      unsigned int i;
      for (i = 0; i < size; i++)
	{
	  // Windows doesn't support hh
	  uint16_t tmp;
	  sscanf(rep_s + 2 * i, "%2hx", &tmp);
	  buffer[i] = tmp;
	}
      fwrite(buffer, size, 1, fp);
    }
  else
    {
      assert(fread(buffer, 1, size, fp) == size);
      if (js) {
	char*		base64 = malloc(2 * size + 1);
	unsigned int	i;
	for (i = 0; i < size; i++)
	  sprintf(base64 + 2 * i, "%.2x", buffer[i]);
	base64[2 * size] = '\0';
	json_object_set_new(js, name, json_string(base64));
      }
    }
}


json_t *js_enter(json_t *parent, const char* name)
{
  if (!parent)
    return NULL;

  json_t *ret = json_object_get(parent, name);
  if (!ret) {
    ret = json_object();
    json_object_set(parent, name, ret);
  }
  return ret;
}

const char* idx_to_str(const char* base, int i)
{
  static char ret[4096];
  sprintf(ret, "%s%d", base, i);
  return ret;
}


void		check_eof(FILE* fp)
{
  long pos = ftell(fp);
  fseek(fp, 0, SEEK_END);
  if (ftell(fp) != pos)
    printf("Error: remaining bytes at the end of the file.\n");
}

int	main(int ac, char** av)
{
  if (ac < 2)
    {
      printf("Usage: %s file.pat [file.js]\n", av[0]);
      return 0;
    }
  
  FILE*	fp;
  json_t* js;

  fp = fopen(av[1], "r+b");
  if (fp == NULL)
    {
      perror(av[1]);
      return 1;
    }

  js = NULL;
  if (av[2]) {
    js = json_load_file(av[2], JSON_ALLOW_NUL, 0);
    if (!js)
      js = json_object();
  }

  part1(fp, av[1], js_enter(js, "part1"));
  textures(fp, av[1], js_enter(js, "textures"));
  take_array(fp, av[1], js_enter(js, "surfaces"));
  part4(fp, av[1], js_enter(js, "part4"));
  check_eof(fp);

  json_dump_file(js, av[2], JSON_INDENT(4));
  fclose(fp);
  return 0;
}
