
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "slice.h"


void Slice_init(Slice *slice) {
  	slice->data_="";
  	slice->size_=0;							//struct init karycha asel tar return karava lagel!!
}

  // Create a slice that refers to d[0,n-1].
void Slice_init_size(Slice *slice,const char* d, size_t n) {
	slice->data_=d;
	slice->size_=n;
}

  // Create a slice that refers to the contents of "s"
void Slice_init_str(Slice *slice,const char *s) {
	slice->data_=s;
	slice->size_=strlen(s);
} 

char *Slice_ToString(Slice *slice) {    
  //return slice->data_[slice->size_];             //seems jhol 
  return (char *)slice->data_;                //added for compiling
}

  // Create a slice that refers to s[0,strlen(s)-1]
  //Slice(const char* s) : data_(s), size_(strlen(s)) { }

  // Return a pointer to the beginning of the referenced data
const char* Slice_data(const Slice *slice) {
	return slice->data_;
}

  // Return the length (in bytes) of the referenced data
size_t Slice_size(const Slice *slice) { 
  	return slice->size_; 
}

  // Return true iff the length of the referenced data is zero
bool Slice_empty(Slice *slice) {
	return slice->size_ == 0;
}

  // Return the ith byte in the referenced data.
  // REQUIRES: n < size()
char Slice_operator_bracket(Slice *slice,size_t n)	{
    assert(n < Slice_size(slice));
    return slice->data_[n];
  }

  // Change this slice to refer to an empty array
  void Slice_clear(Slice *slice) { 
  	slice->data_="";
  	slice->size_=0;
}
  // Drop the first "n" bytes from this slice.
 void Slice_remove_prefix(Slice *slice,size_t n) {
    assert(n <= Slice_size(slice));
    slice->data_ += n;
    slice->size_ -= n;
  }

  // Return a string that contains the copy of the referenced data.
/*char *Slice_ToString(Slice *slice) { 			//jhol
	return strcpy(data_, size_); 
}
*/
  // Three-way comparison.  Returns value:
  //   <  0 iff "*this" <  "b",
  //   == 0 iff "*this" == "b",
  //   >  0 iff "*this" >  "b"
  //int compare(const Slice& b) const;

  // Return true iff "x" is a prefix of "*this"
  bool Slice_starts_with(Slice *slice,const Slice *x) {
    return ((slice->size_ >= x->size_) &&
            (memcmp(slice->data_, x->data_, x->size_) == 0));
  }

 //private:
  

  // Intentionally copyable
//};

bool Slice_operator_equals(const Slice *x, const Slice *y) {
  return ((Slice_size(x) == Slice_size(y)) &&
          (memcmp((const void*)Slice_data(x),(const void*) Slice_data(y), Slice_size(x)) == 0));
}

inline bool Slice_operator_not_equals(const Slice *x, const Slice *y) {
  return !(x == y);
}

inline int Slice_compare(Slice *slice,const Slice *	b) {
  const int min_len = (slice->size_ < b->size_) ? slice->size_ : b->size_;
  int r = memcmp(slice->data_, b->data_, min_len);
  if (r == 0) {
    if (slice->size_ < b->size_) r = -1;
    else if (slice->size_ > b->size_) r = +1;
  }
  return r;
}

//}  // namespace leveldb
