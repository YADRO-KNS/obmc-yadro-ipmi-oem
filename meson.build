project('obmc-yadro-ipmi-oem', 'cpp',
    default_options: [
        'cpp_std=c++20',
        'warning_level=3',
        'werror=true',
    ],
    meson_version: '>=0.57.0',
    license: 'Apache-2.0',
    version: '0.1',
)

cpp = meson.get_compiler('cpp')
add_project_arguments(
  cpp.get_supported_arguments([
    '-DBOOST_ERROR_CODE_HEADER_ONLY',
    '-DBOOST_SYSTEM_NO_DEPRECATED',
    '-DBOOST_COROUTINES_NO_DEPRECATION_WARNING',
    '-DBOOST_ASIO_DISABLE_THREADS',
    '-DBOOST_ALL_NO_LIB',
  ]),
  language : 'cpp')

phosphor_logging_dep = dependency('phosphor-logging')
pdi_dep = dependency('phosphor-dbus-interfaces')
phosphor_ipmi_host_dep = dependency('libipmid')
sdeventplus_dep = dependency('sdeventplus')
systemd = dependency('systemd')
boost_coroutine = cpp.find_library('boost_coroutine', required: true)
sdbusplus_dep = dependency('sdbusplus')

incdir = include_directories('src', 'include')

shared_library('yadroipmioem', 
    'src/oem_cmd.cpp',
    'src/inventory_cmd.cpp',
    include_directories : incdir,
    implicit_include_directories: false,
    dependencies: [
        boost_coroutine,
        systemd,
        sdbusplus_dep,
        phosphor_logging_dep,
        phosphor_ipmi_host_dep,
        pdi_dep,
    ],
    version: meson.project_version(),
    override_options: ['b_lundef=false'],
    install: true,
    install_dir: get_option('libdir') / 'ipmid-providers')

