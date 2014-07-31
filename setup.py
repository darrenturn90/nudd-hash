from distutils.core import setup, Extension

nudd_hash_module = Extension('nudd_hash',
                               sources = ['nuddmodule.cpp',
                                          'bcrypt.cpp'])

setup (name = 'nudd_hashs',
       version = '1.0',
       description = 'Bindings for proof of work used by NUDD',
       ext_modules = [nudd_hash_module])
