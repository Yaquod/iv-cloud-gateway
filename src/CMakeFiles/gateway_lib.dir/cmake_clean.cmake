file(REMOVE_RECURSE
  "libgateway_lib.a"
  "libgateway_lib.pdb"
)

# Per-language clean rules from dependency scanning.
foreach(lang CXX)
  include(CMakeFiles/gateway_lib.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
