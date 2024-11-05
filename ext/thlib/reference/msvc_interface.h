/* Copyright 2011 <happyteam@thinkwaresys.com> */
#ifndef _MSVC_INTERFACE_H__
#define _MSVC_INTERFACE_H__



/**
 *  This file is to adapt to the interface of msvc6helpers.
 *  The same namespace and class name is given
 *    , so that you can use same source in windows and linux.
 *  Pay attention to keep from using both this file and msvc6helpers file.
 *
 *  If you have some questions for this file
 *    , you might want to ask Nineye.
 *  But he may don't know about this, don't expect to him.
 *
 */

namespace Loki {

  namespace Private {

    template <class TypeWithNestedTemplate>
    struct ApplyImpl1 {
      enum { alwaysfalse = false };

      /**
       * Make reference to the MSVC6_Helpers.h
       *
       * There were several things for M$ compiler's inability in here.
       * For example, VC_WORDAROUND, AlwaysFalse and etc...
       * Gcc compiler don't need those, so I removed them.
       * The interfaces remain only to adapt to M$'s interfaces.
       */

      template<class T1>
      struct Result : public TypeWithNestedTemplate::template In<T1> {
        typedef typename TypeWithNestedTemplate::template In<T1>::type Base;
      };
    };  //  end of struct ApplyImpl1
  };  //  end of namespace Loki::Private


  template<class F, class T1>
  struct Apply1 : ::Loki::Private::ApplyImpl1<F>::template Result<T1> {
    typedef typename 
      ::Loki::Private::ApplyImpl1<F>::template Result<T1>::Base Base;
  };
};  //  end of namespace Loki


#endif  //  end of #ifndef _MSVC_INTERFACE_H__
