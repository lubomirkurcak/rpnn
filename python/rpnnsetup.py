from distutils.core import setup, Extension

module1 = Extension('rpnn',
                     sources = ['rpnnmodule.cpp'],
                     include_dirs = ['../code', './code'],
                     extra_compile_args=['/Zi', '/Od'],
                     extra_link_args=['/DEBUG'])

setup (name = 'ReducedPrecisionNeuralNet',
       version = '1.0',
       description = 'This is a spam demo package',
       ext_modules = [module1])
