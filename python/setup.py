from distutils.core import setup, Extension

setup(name='pykustomer', version='1.0',
      ext_modules=[
        Extension('_pykustomer',
                  ['pykustomer.c'],
                  include_dirs=['../.libs/include/kustomer'],
                  library_dirs=['../.libs'],
                  libraries=['kustomer'])
      ],
      packages=['pykustomer'],
      package_dir={'pykustomer': 'src'},
      )
