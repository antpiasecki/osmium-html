project(
    'osmium-html',
    'cpp',
    version: '0.1',
    default_options: ['warning_level=3', 'cpp_std=c++20'],
)

libosmium_html = static_library(
    'osmium-html',
    sources: ['src/tokenizer.cc', 'src/parser.cc'],
    include_directories: include_directories('include/osmium-html'),
    cpp_args: ['-Wall', '-Wextra', '-Wpedantic', '-Wconversion'],
)

libosmium_html_dep = declare_dependency(
    link_with: libosmium_html,
    include_directories: include_directories('include'),
)