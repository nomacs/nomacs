# Update Exiv2 🍸

in order to create the msvc solution you can safely delete all projects except for:
- exiv2lib
- expat
- xmpsdk
- zlib1
- in addition, the build targets ``Release`` and ``Debug`` can be safely deleted too.

If you get an error which is similar to this:
```
DkMetaData.obj : error LNK2019: unresolved external symbol "public: static class std::auto_ptr<class Exiv2::Image> __cdecl Exiv2::ImageFactory::open(class std::basic_string<unsigned short,struct std::char_traits<unsigned short>,class std::allocator<unsigned short> > const &)" (?open@ImageFactory@Exiv2@SA?AV?$auto_ptr@VImage@Exiv2@@std@ABV?$basic_string@GU?$char_traits@G@std@V?$allocator@G@2@4@Z) referenced in function "public: void __thiscall nmc::DkMetaDataT::readMetaData(class QFileInfo const &,class QSharedPointer<class QByteArray>)" (?readMetaData@DkMetaDataT@nmc@QAEXABVQFileInfo@V?$QSharedPointer@VQByteArray@@@Z)
C:\VSProjects\nomacs\build2012x86\Debug\libnomacsd.dll : fatal error LNK1120: 1 unresolved externals
```
you have to open preferences _(of the exiv2lib project) > C/C++ > Language_ and make sure that ``Treat WChar_t`` as Built in Type is set to ``No (/Z:wchar_t-)``
