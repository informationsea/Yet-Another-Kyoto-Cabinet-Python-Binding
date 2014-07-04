from distutils.core import setup, Extension
import os.path

kyotocabinet_cc = "kcutil.cc", "kcthread.cc", "kcfile.cc", "kccompress.cc", "kccompare.cc", "kcmap.cc", "kcregex.cc", "kcdb.cc", "kcplantdb.cc", "kcprotodb.cc", "kcstashdb.cc", "kccachedb.cc", "kchashdb.cc", "kcdirdb.cc", "kctextdb.cc", "kcpolydb.cc", "kcdbext.cc", "kclangc.cc"

module1 = Extension('yakc',
                    include_dirs = ['kyotocabinet-1.2.76'],
                    sources = ['yakc.cpp'] + [os.path.join('kyotocabinet-1.2.76', x) for x in kyotocabinet_cc])

setup (name = 'yakyotocabinet',
       version = '0.1',
       description = 'Yet Another Kyoto Cabinet Binding',
       author = 'OKAMURA Yasunobu',
       author_email = 'okamura@informationsea.info',
       ext_modules = [module1],
       url = 'https://github.com/informationsea/Yet-Another-Kyoto-Cabinet-Python-Binding')
