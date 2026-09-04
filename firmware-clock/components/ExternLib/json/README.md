# json compatibility adapter

ESP-IDF 6 removed the built-in `json` component while ESP-SR 2.3.x still
declares it as a dependency. This local, source-free adapter supplies that
legacy component name and forwards its public dependency to the managed
`espressif/cjson` component. It is added only when `IDF_VERSION_MAJOR >= 6`.
