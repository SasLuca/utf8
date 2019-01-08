#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <cstdarg>

// only using 3 functions so avoiding including the whole header and poluting namespace
// #define UTF8PROC_STATIC
// #include "../../extern/utf8proc/utf8proc.h"



namespace utf8 {
  extern void setup_console ();

  extern size_t char_size (uint8_t const* c);

  extern size_t char_size (int32_t c);

  extern
  int32_t to_int (uint8_t const* c);

  extern
  size_t encode (int32_t c, uint8_t* bytes);

  extern
  size_t put_char (uint8_t const* ustr, FILE* f);

  extern
  size_t put_char (int32_t c, FILE* f);

  extern
  size_t get_char (uint8_t* ustr, FILE* f);

  extern
  int32_t get_char (FILE* f);

  extern
  uint8_t* index_offset (uint8_t* ustr, size_t index);

  extern
  uint8_t const* index_offset (uint8_t const* ustr, size_t index);

  extern
  size_t byte_offset (uint8_t const* ustr, size_t index);

  extern
  size_t char_count (uint8_t const* ustr);
  
  inline
  size_t byte_count (uint8_t const* ustr) {
    return strlen((char const*) ustr);
  }

  extern
  size_t char_at (uint8_t const* ustr, size_t index);



  struct StringIteratorResult {
    size_t i;
    int32_t v;
  };

  struct StringIterator {
    size_t index = 0;
    uint8_t const* bytes = NULL;
    
    StringIteratorResult operator * () const;

    StringIterator& operator ++ ();

    bool operator != (StringIterator const& other) const;
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


    ~String () {
      dispose();
    }


    StringIterator begin () const {
      return { 0, bytes };
    }

    StringIterator end () const {
      return { utf8::char_count(bytes), bytes + byte_length };
    }


    operator char* () const { return (char*) bytes; }
    operator char const* () const { return (char const*) bytes; }

    operator uint8_t* () const { return bytes; }
    operator uint8_t const* () const { return bytes; }


    bool operator == (uint8_t const* other) const {
      return strcmp((char const*) bytes, (char const*) other) == 0;
    }

    bool operator != (uint8_t const* other) const {
      return strcmp((char const*) bytes, (char const*) other) != 0;
    }
    
    
    int32_t operator [] (size_t index) const {
      return char_at(index);
    }


    size_t length () const {
      return utf8::char_count(bytes);
    }

    int32_t char_at (size_t index) const {
      return utf8::char_at(bytes, index);
    }

    uint8_t* str_at (size_t index) const {
      return utf8::index_offset(bytes, index);
    }


    void dispose ();

    uint8_t* release ();

    void move (String& source);

    
    static
    String from_file (char const* file_name);

    void to_file (char const* file_name) const;


    void collapse_allocation ();

    void grow_allocation (size_t additional_length);
  

    void insert (uint8_t const* str, size_t length = 0);

    void insert (char const* str, size_t length = 0);

    void insert (int32_t c);


    void insert_at (size_t index, uint8_t const* seg, size_t length = 0);
    
    void insert_at (size_t index, char const* str, size_t length = 0);

    void insert_at (size_t index, int32_t c);


    void remove (size_t index, size_t count = 1);


    void insert_fmt_va (uint8_t const* fmt, va_list args);

    void insert_fmt_va (char const* fmt, va_list args);

    void insert_fmt (uint8_t const* fmt, ...);

    void insert_fmt (char const* fmt, ...);
    

    void insert_fmt_at_va (size_t index, uint8_t const* fmt, va_list args);

    void insert_fmt_at_va (size_t index, char const* fmt, va_list args);

    void insert_fmt_at (size_t index, uint8_t const* fmt, ...);

    void insert_fmt_at (size_t index, char const* fmt, ...);

       
    String to_lowercase () const;

    String to_uppercase () const;

    // utf8proc totitle is broken
    // String to_titlecase () const;

    String casefold () const;
  };
}