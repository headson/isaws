#ifndef VZBASE_BASE_NONCOPYABLE_H_
#define VZBASE_BASE_NONCOPYABLE_H_

namespace vzbase { // protection from unintended ADL
class noncopyable {
 protected:
//#if !defined(BOOST_NO_CXX11_DEFAULTED_FUNCTIONS) && !defined(BOOST_NO_CXX11_NON_PUBLIC_DEFAULTED_FUNCTIONS)
//      BOOST_CONSTEXPR noncopyable() = default;
//      ~noncopyable() = default;
//#else
  noncopyable() {}
  ~noncopyable() {}
//#endif
//#if !defined(BOOST_NO_CXX11_DELETED_FUNCTIONS)
//      noncopyable( const noncopyable& ) = delete;
//      noncopyable& operator=( const noncopyable& ) = delete;
//#else
 private:  // emphasize the following members are private
  noncopyable( const noncopyable& );
  noncopyable& operator=( const noncopyable& );
// #endif
};
}

#endif  // VZBASE_BASE_NONCOPYABLE_H_
