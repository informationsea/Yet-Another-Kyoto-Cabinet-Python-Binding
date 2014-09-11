from distutils.core import setup, Extension
import os.path

kyotocabinet_cc = "kcutil.cc", "kcthread.cc", "kcfile.cc", "kccompress.cc", "kccompare.cc", "kcmap.cc", "kcregex.cc", "kcdb.cc", "kcplantdb.cc", "kcprotodb.cc", "kcstashdb.cc", "kccachedb.cc", "kchashdb.cc", "kcdirdb.cc", "kctextdb.cc", "kcpolydb.cc", "kcdbext.cc", "kclangc.cc"

#kyotocabinet_cc = []

README = open(os.path.join(os.path.dirname(__file__), 'README.rst')).read()

module1 = Extension('yakc',
                    include_dirs = ['kyotocabinet-1.2.76'],
                    sources = ['yakc.cpp'] + [os.path.join('kyotocabinet-1.2.76', x) for x in kyotocabinet_cc],
                    libraries=[])

setup (name = 'yakc',
       version = '0.1.5',
       description = 'Yet Another Kyoto Cabinet Binding',
       author = 'OKAMURA Yasunobu',
       author_email = 'okamura@informationsea.info',
       long_description=README,
       ext_modules = [module1],
       license = 'GPL3',
       url = 'https://github.com/informationsea/Yet-Another-Kyoto-Cabinet-Python-Binding',
       classifiers=[
        'Intended Audience :: Developers',
        'License :: OSI Approved :: GNU General Public License v3 or later (GPLv3+)',
        'Programming Language :: Python :: 2.7',
       ] 
       )
