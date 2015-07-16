#! /usr/bin/env python
# encoding: utf-8
# Calaos, 2011

VERSION='1.1.8'
APPNAME='calaos_tools'

top = '..'
out = 'build'

def options(opt):
        opt.load('compiler_c')
        opt.load('compiler_cxx')
        opt.add_option('--curl_prefix', type='string', help='Prefix to find curl-config', dest='curldir')

def configure(conf):
        import os

        conf.check_waf_version(mini='1.6.2')
        conf.load('compiler_c')
        conf.load('compiler_cxx')

        conf.check_cfg(atleast_pkgconfig_version='0.2')

        conf.check_cfg(package='sigc++-2.0', uselib_store='SIGC', args='--libs --cflags')
        conf.check_cfg(package='ecore', args=['ecore >= 1.0.0', '--libs', '--cflags'])
        conf.check_cfg(package='ecore-file', args='--libs --cflags')
        conf.check_cfg(package='ecore-con', args='--libs --cflags')
        conf.check_cfg(package='log4cpp', args='--libs --cflags')
        conf.check_cfg(package='jansson', args='--libs --cflags')

        conf.check_cxx(lib='pthread', use='PTHREAD', cxxflags='-O2', mandatory=True)

        if conf.options.curldir:
                curl_config = conf.options.curldir + '/curl-config'
        else:
                curl_config = 'curl-config'

        conf.check_cfg(path=curl_config, args='--cflags --libs', package='', uselib_store='CURL')

        conf.define('VERSION', VERSION)

        conf.env['CFLAGS']=['-O2', '-g', '-W', '-Wall']
        conf.env['CCFLAGS']=['-O2', '-g', '-W', '-Wall']
        conf.env['CXXFLAGS']=['-g']

        conf.write_config_header('config.h')


def build(bld):
     
        bld.program(
        features='cxx cxxprogram',
        source = '''
        wago_simulator/main.cpp
        wago_simulator/modbus.c
        wago_simulator/modbus-data.c
        wago_simulator/modbus-rtu.c
        wago_simulator/modbus-tcp.c
        ''',
        includes = '''
        .
        ..
        wago_simulator
        ../calaos_common
        ''',
        uselib = 'CURL ECORE ECORE-CON',
        target = 'wago_sim'
        )

