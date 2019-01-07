#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <cstdarg>

// only using 3 functions so avoiding including the whole header and poluting namespace
// #define UTF8PROC_STATIC
// #include "../../extern/utf8proc/utf8proc.h"


#ifdef _WIN32
  namespace Windows {
    extern "C" {
      int __declspec(dllimport) IsValidCodePage (unsigned int);
      int __declspec(dllimport) SetConsoleCP (unsigned int);
      int __declspec(dllimport) SetConsoleOutputCP (unsigned int);
    }
    
    static constexpr
    unsigned int UTF8_FLAG = 65001;
  }
#endif


namespace utf8 {
  extern "C" {
    int32_t  utf8proc_toupper(int32_t c);
    int32_t  utf8proc_tolower(int32_t c);
    uint8_t* utf8proc_NFKC_Casefold(uint8_t const* str);
  }

  inline
  void setup_console () {
    #ifdef _WIN32
      if (Windows::IsValidCodePage(Windows::UTF8_FLAG)) {
        if (!Windows::SetConsoleCP(Windows::UTF8_FLAG)) goto err;
        if (!Windows::SetConsoleOutputCP(Windows::UTF8_FLAG)) goto err;
        return;
      }

      err: {
        printf("A UTF-8 compatible terminal is required\n");
        abort();
      }
    #endif
  }

  inline
  size_t char_size (uint8_t const* c) {
    if (*c < 128) {
      return 1;
    } else if (*c < 224) {
      return 2;
    } else if (*c < 240) {
      return 3;
    } else {
      return 4;
    }
  }

  inline
  size_t char_size (int32_t c) {
    if (c < 0) goto err;
    else if (c < 128) {
      return 1;
    } else if (c < 2048) {
      return 2;
    } else if (c < 65536) {
      return 3;
    } else if (c < 1114112) {
      return 4;
    } else goto err;

    err: {
      printf("Char code %d is out of utf8 range (Must be integer 0 - 1114112)\n", c);
      abort();
    }
  }

  inline
  int32_t to_int (uint8_t const* c) {
    int32_t out = *c;

    if (out < 128) return out;
    else if (out < 224) return ((out & 31) << 6) | (c[1] & 63);
    else if (out < 240) return ((out & 15) << 12) | ((c[1] & 63) << 6) | (c[2] & 63);
    else return ((out & 7) << 18) | ((c[1] & 63) << 12) | ((c[2] & 63) << 6) | (c[3] & 63);
  }

  inline
  size_t encode (int32_t c, uint8_t* bytes) {
    size_t length = char_size(c);

    if (length == 1) {
      bytes[0] = c;
    } else if (length == 2) {
      bytes[0] = 192 + (c >> 6);
      bytes[1] = 128 + (c & 63);
    } else if (length == 3) {
      bytes[0] = 224 + (c >> 12);
      bytes[1] = 128 + ((c >> 6) & 63);
      bytes[2] = 128 + (c & 63);
    } else if (length == 4) {
      bytes[0] = 240 + (c >> 18);
      bytes[1] = 128 + ((c >> 12) & 63);
      bytes[2] = 128 + ((c >> 6) & 63);
      bytes[3] = 128 + (c & 63);
    }

    return length;
  }

  inline
  size_t put_char (uint8_t const* ustr, FILE* f = stdout) {
    size_t adv = char_size(ustr);
    fwrite(ustr, adv, 1, f);
    return adv;
  }

  inline
  size_t put_char (int32_t c, FILE* f) {
    int32_t mem = 0;
    encode(c, (uint8_t*) &mem);
    return put_char((uint8_t const*) &mem, f);
  }

  inline
  size_t get_char (uint8_t* ustr, FILE* f) {
    if (!fread(ustr, 1, 1, f)) return 0;

    size_t size = char_size(ustr);

    if (size > 1) {
      if (!fread(ustr + 1, size - 1, 1, f)) {
        printf("Unexpected EOF: File ends in the middle of utf8 character code\n");
        abort();
      }
    }

    return size;
  }

  inline
  int32_t get_char (FILE* f) {
    int32_t mem = 0;
    get_char((uint8_t*) &mem, f);
    return to_int((uint8_t*) &mem);
  }
  
  template <typename T>
  inline
  T __char_iterate (T ustr, size_t index) {
    T o = ustr;

    for (size_t i = 0; i < index; i ++) {
      if (*o == '\0') {
        printf("Out of range utf8 character index %zu for string '%s', max index is %zu\n", index, ustr, i);
        abort();
      }

      o += char_size(o);
    }

    return o;
  }

  static
  uint8_t* index_offset (uint8_t* ustr, size_t index) {
    return __char_iterate(ustr, index);
  }

  static
  uint8_t const* index_offset (uint8_t const* ustr, size_t index) {
    return __char_iterate(ustr, index);
  }

  static
  size_t byte_offset (uint8_t const* ustr, size_t index) {
    return (size_t) index_offset(ustr, index) - (size_t) ustr;
  }

  static
  size_t char_count (uint8_t const* ustr) {
    size_t i = 0;

    while (*ustr != '\0') {
      ++ i;
      ustr += char_size(ustr);
    }

    return i;
  }
  
  inline
  size_t byte_count (uint8_t const* ustr) {
    return strlen((char const*) ustr);
  }

  inline
  size_t char_at (uint8_t const* ustr, size_t index) {
    return to_int(index_offset(ustr, index));
  }



  struct StringIteratorResult {
    size_t i;
    int32_t v;
  };

  struct StringIterator {
    size_t index = 0;
    uint8_t const* bytes = NULL;
    
    StringIteratorResult operator * () const {
      return { index, utf8::to_int(bytes) };
    }

    StringIterator& operator ++ () {
      index ++;
      bytes += utf8::char_size(bytes);
      return *this;
    }

    bool operator != (StringIterator const& other) const {
      return index != other.index;
    }
  };


  struct String {
    static constexpr size_t DEFAULT_CAPACITY = 16;

    uint8_t* bytes = NULL;
    size_t byte_length = 0;
    size_t byte_capacity = 0;


    String () = default;

    String (size_t init_capacity)
    { grow_allocation(init_capacity); }

    String (uint8_t const* src)
    { insert(src); }

    String (char const* src)
    { insert((uint8_t const*) src); }

    String (uint8_t const* src, size_t length)
    { insert(src, length); }

    String (String const& src)
    { insert(src.bytes, src.byte_length); }

    String (uint8_t* in_bytes, size_t in_byte_length, size_t in_byte_capacity)
    : bytes(in_bytes)
    , byte_length(in_byte_length)
    , byte_capacity(in_byte_capacity)
    { }


    StringIterator begin () {
      return { 0, bytes };
    }

    StringIterator end () {
      return { utf8::char_count(bytes), bytes + byte_length };
    }


    operator char* () { return (char*) bytes; }
    operator char const* () { return (char const*) bytes; }

    operator uint8_t* () { return bytes; }
    operator uint8_t const* () { return bytes; }


    bool operator == (uint8_t const* other) {
      return strcmp((char const*) bytes, (char const*) other) == 0;
    }

    bool operator != (uint8_t const* other) {
      return strcmp((char const*) bytes, (char const*) other) != 0;
    }
    
    
    int32_t operator [] (size_t index) {
      return char_at(index);
    }


    void dispose () {
      if (bytes != NULL) {
        free(bytes);
        bytes = NULL;
      }

      byte_length = 0;
      byte_capacity = 0;
    }

    uint8_t* release () {
      collapse_allocation();

      uint8_t* p = bytes;

      bytes = NULL;
      dispose();

      return p;
    }


    ~String () {
      dispose();
    }


    static
    String from_file (char const* file_name) {
      FILE* f = NULL;

      if (fopen_s(&f, file_name, "rb")) {
        printf("Error reading file \"%s\"\n", file_name);
        abort();
      }

      fseek(f, 0, SEEK_END);

      size_t length = ftell(f);

      fseek(f, 0, SEEK_SET);

      String out { length };

      fread(out.bytes, length, 1, f);

      fclose(f);

      out.bytes[length] = 0;

      return out;
    }

    void to_file (char const* file_name) {
      FILE* f = NULL;

      if (fopen_s(&f, file_name, "wb")) {
        printf("Error writing file \"%s\"\n", file_name);
        abort();
      }

      fwrite(bytes, byte_length, 1, f);

      fclose(f);
    }



    size_t length () {
      return utf8::char_count(bytes);
    }

    int32_t char_at (size_t index) {
      return utf8::char_at(bytes, index);
    }

    uint8_t* str_at (size_t index) {
      return utf8::index_offset(bytes, index);
    }


    void collapse_allocation () {
      if (byte_capacity > byte_length) {
        bytes = (uint8_t*) realloc(bytes, byte_length);
        byte_capacity = byte_length;
      }
    }

    void grow_allocation (size_t additional_length) {
      size_t required_capacity = byte_length + additional_length + 1;

      size_t new_capacity = byte_capacity > 0? byte_capacity : DEFAULT_CAPACITY;

      while (new_capacity < required_capacity) new_capacity *= 2;

      if (new_capacity > byte_capacity) {
        byte_capacity = new_capacity;

        if (bytes == NULL) bytes = (uint8_t*) malloc(byte_capacity);
        else bytes = (uint8_t*) realloc(bytes, byte_capacity);

        if (bytes == NULL) {
          printf("Out of memory or other null pointer error while growing String allocation\n");
          abort();
        }
      }
    }
  

    void insert (uint8_t const* str, size_t length = 0) {
      if (length == 0) length = utf8::byte_count(str);

      grow_allocation(length);

      memcpy(bytes + byte_length, str, length);

      byte_length += length;

      bytes[byte_length] = 0;
    }

    void insert (char const* str, size_t length = 0) {
      insert((uint8_t const*) str, length);
    }

    void insert (int32_t c) {
      size_t length = utf8::char_size(c);

      grow_allocation(length);

      if (length == 1) {
        bytes[byte_length ++] = c;
      } else if (length == 2) {
        bytes[byte_length ++] = 192 + (c >> 6);
        bytes[byte_length ++] = 128 + (c & 63);
      } else if (length == 3) {
        bytes[byte_length ++] = 224 + (c >> 12);
        bytes[byte_length ++] = 128 + ((c >> 6) & 63);
        bytes[byte_length ++] = 128 + (c & 63);
      } else if (length == 4) {
        bytes[byte_length ++] = 240 + (c >> 18);
        bytes[byte_length ++] = 128 + ((c >> 12) & 63);
        bytes[byte_length ++] = 128 + ((c >> 6) & 63);
        bytes[byte_length ++] = 128 + (c & 63);
      }

      bytes[byte_length] = 0;
    }


    void insert_at (size_t index, uint8_t const* seg, size_t length = 0) {
      if (index >= utf8::char_count(bytes)) return insert(seg, length);

      if (length == 0) length = utf8::byte_count(seg);
    
      grow_allocation(length);

      size_t offset = utf8::byte_offset(bytes, index);

      memmove(bytes + offset + length, bytes + offset, byte_length - offset);

      memcpy(bytes + offset, seg, length);

      byte_length += length;
    }
    
    void insert_at (size_t index, char const* str, size_t length = 0) {
      insert_at(index, (uint8_t const*) str, length);
    }

    void insert_at (size_t index, int32_t c) {
      if (index >= utf8::char_count(bytes)) return insert(c);

      size_t length = utf8::char_size(c);
      
      grow_allocation(length);

      size_t offset = utf8::byte_offset(bytes, index);

      memmove(bytes + offset + length, bytes + offset, byte_length - offset);
      
      if (length == 1) {
        bytes[offset] = c;
      } else if (length == 2) {
        bytes[offset + 0] = 192 + (c >> 6);
        bytes[offset + 1] = 128 + (c & 63);
      } else if (length == 3) {
        bytes[offset + 0] = 224 + (c >> 12);
        bytes[offset + 1] = 128 + ((c >> 6) & 63);
        bytes[offset + 2] = 128 + (c & 63);
      } else if (length == 4) {
        bytes[offset + 0] = 240 + (c >> 18);
        bytes[offset + 1] = 128 + ((c >> 12) & 63);
        bytes[offset + 2] = 128 + ((c >> 6) & 63);
        bytes[offset + 3] = 128 + (c & 63);
      }

      byte_length += length;
    }


    void remove (size_t index, size_t count = 1) {
      size_t base = utf8::byte_offset(bytes, index);
      size_t end = utf8::byte_offset(bytes, index + count);

      memmove(bytes + base, bytes + end, byte_length - end);

      byte_length -= end - base;
    }


    void insert_fmt_va (uint8_t const* fmt, va_list args) {
      va_list args_b;

      va_copy(args_b, args);

      int length = vsnprintf(NULL, 0, (char const*) fmt, args_b);
      
      va_end(args_b);

      grow_allocation(length);

      vsnprintf((char*) bytes + byte_length, length + 1, (char const*) fmt, args);

      byte_length += length;
      bytes[byte_length] = 0;
    }

    void insert_fmt_va (char const* fmt, va_list args) {
      insert_fmt_va((uint8_t const*) fmt, args);
    }

    void insert_fmt (uint8_t const* fmt, ...) {
      va_list args;
      va_start(args, fmt);
      insert_fmt_va(fmt, args);
      va_end(args);
    }

    void insert_fmt (char const* fmt, ...) {
      va_list args;
      va_start(args, fmt);
      insert_fmt_va(fmt, args);
      va_end(args);
    }
    

    void insert_fmt_at_va (size_t index, uint8_t const* fmt, va_list args) {
      if (index >= utf8::char_count(bytes)) return insert_fmt_va(fmt, args);

      va_list args_b;

      va_copy(args_b, args);

      int length = vsnprintf(NULL, 0, (char const*) fmt, args_b);
      
      va_end(args_b);

      grow_allocation(length + 1);

      size_t offset = utf8::byte_offset(bytes, index);

      uint8_t* ptr = bytes + offset;

      uint8_t* dest = ptr + length + 1;
      uint8_t* dest1 = dest - 1;
      size_t move_length = byte_length - offset;
      memmove(dest, ptr, move_length); // this double move really sucks but vsnprintf writes a null terminator

      vsnprintf((char*) ptr, length + 1, (char const*) fmt, args);
      memmove(dest1, dest, move_length);

      byte_length += length;
      bytes[byte_length] = 0;
    }

    void insert_fmt_at_va (size_t index, char const* fmt, va_list args) {
      insert_fmt_at_va(index, (uint8_t const*) fmt, args);
    }

    void insert_fmt_at (size_t index, uint8_t const* fmt, ...) {
      va_list args;
      va_start(args, fmt);
      insert_fmt_at_va(index, fmt, args);
      va_end(args);
    }

    void insert_fmt_at (size_t index, char const* fmt, ...) {
      va_list args;
      va_start(args, fmt);
      insert_fmt_at_va(index, fmt, args);
      va_end(args);
    }

       
    String to_lowercase () {
      String out { byte_length };

      for (auto [ i, c ] : *this) out.insert(utf8::utf8proc_tolower(c));

      return out;
    }

    String to_uppercase () {
      String out { byte_length };

      for (auto [ i, c ] : *this) out.insert(utf8::utf8proc_toupper(c));

      return out;
    }

    // utf8proc totitle is broken
    // String to_titlecase () { 
    //   String out { byte_length };

    //   for (auto [ i, c ] : *this) out.insert(utf8proc_totitle(c));

    //   return out;
    // }

    String casefold () {
      uint8_t* new_bytes = utf8::utf8proc_NFKC_Casefold(bytes);
      
      size_t new_length = utf8::byte_count(new_bytes);
      
      size_t new_capacity = DEFAULT_CAPACITY;
      while (new_capacity < new_length) new_capacity *= 2;

      if (new_capacity > new_length) new_bytes = (uint8_t*) realloc(new_bytes, new_capacity);

      return { new_bytes, new_length, new_capacity };
    }
  };
}