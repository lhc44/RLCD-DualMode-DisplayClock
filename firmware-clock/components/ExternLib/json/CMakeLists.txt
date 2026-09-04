# ESP-IDF 6 compatibility adapter for legacy components that still require
# the removed built-in component name `json` (notably ESP-SR 2.3.x).
idf_component_register(INCLUDE_DIRS "." REQUIRES espressif__cjson)
