from building import *
Import('rtconfig')

src   = []
cwd   = GetCurrentDir()

# add bq32k src files.
if GetDepend('PKG_USING_BQ32K'):
    src += Glob('bq32k.c')

if GetDepend('PKG_USING_BQ32K_SAMPLE'):
    src += Glob('bq32k_sample.c')

# add bq32k include path.
path  = [cwd]

# add src and include to group.
group = DefineGroup('bq32k', src, depend = ['PKG_USING_BQ32K'], CPPPATH = path)

Return('group')
