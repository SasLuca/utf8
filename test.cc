#include "utf8.hh"



int main() {
  utf8::setup_console();


  auto unicode_text = (uint8_t const*) "!حسنا رائعaäbcdefghijklmnoöpqrsßtuüvwxyzいいかっこいい！😊";
  size_t size = strlen((char const*) unicode_text);

  printf("\"%s\" %zu\n", unicode_text, size);
  

  size_t i = 0;
  size_t j = 0;
  while (i < size) {
    size_t csize = utf8::char_size(unicode_text + i);
    int32_t c = utf8::to_int(unicode_text + i);
    printf("%zu: ", j ++);
    printf("'%.*s' (%zu -> %zu) ", (int) csize, (char const*) unicode_text + i, i, csize);
    utf8::put_char(c, stdout);
    putchar('\n');
    i += csize;
  }


  utf8::put_char(utf8::index_offset(unicode_text, 48), stdout);
  putchar(' ');


  auto x = (uint8_t const*) "llama💩llama";
  utf8::put_char(utf8::index_offset(x, 5), stdout);

  printf("  %zu\n", utf8::char_count(x));


  utf8::String string0 { "hello😊world" };
  string0.insert_at(0, "llama💩");
  string0.insert_at(6, "drama ");
  string0.insert_fmt_at(12, "big ol testo, %d, %f, ", 100, 50.5f);
  string0.insert_fmt(" large literal %p\n", &string0);
  printf("Test string0: %s", (char*) string0);

  utf8::String string1 { "BIG LETTERS ÀÈÌÒÙ ÁÉÍÓÚÝ" };
  utf8::String string2 { "lil letters àèìòù áéíóúý" };

  printf("to_lower:\n%s\n%s\n\n", (char*) string1.to_lowercase(), (char*) string2.to_lowercase());

  printf("to_upper:\n%s\n%s\n\n", (char*) string1.to_uppercase(), (char*) string2.to_uppercase());
  
  // utf8proc totitle is broken
  // printf("to_title:\n%s\n%s\n\n", (char*) string1.to_titlecase(), (char*) string2.to_titlecase());

  printf("casefold:\n%s\n%s\n\n", (char*) string1.casefold(), (char*) string2.casefold());


  utf8::String string3 { u8"Ñoo"};
  string3.insert(u'ß');
  string3.insert(0x00df);
  string3.insert('s');
  printf("string3: '%s'\n", (char*) string3);

  int32_t mem;
  for (auto [ i, c ] : string3) {
    size_t l = utf8::encode(c, (uint8_t*) &mem);
    printf("%zu:%d:%.*s\n", i, c, (int) l, (char*) &mem);
  }


  FILE* f;
  #ifdef _WIN32
    f = NULL;
    fopen_s(&f, "test_in.txt", "rb");
  #else
    f = fopen("test_in.txt", "rb");
  #endif

  if (f == NULL) {
    printf("Error reading file \"test_in.txt\"\n");
    abort();
  }

  uint8_t cmem [4] = { 0, 0, 0, 0 };
  size_t csize = 0;
  while ((csize = utf8::get_char(cmem, f)) != 0) {
    printf("Read char from file: '%.*s'\n", (int) csize, (char*) cmem);
  }

  fclose(f);


  utf8::String string4 = utf8::String::from_file("test_in.txt");
  printf("Read file to utf8::String: '%s'\n", (char*) string4);


  utf8::String string5 { "Hello world 😊\nllama llama llama 💩\ndrÀmÀ drÀmÀ drÀmÀ\nÑooß" };
  string5.to_file("test_out.txt");
  printf("Wrote file to test_out.txt\n");

  printf("Number of columns for 😊: %zu, for A: %zu\n", utf8::column_count((uint8_t const*) "😊"), utf8::column_count((int32_t)'A'));
}