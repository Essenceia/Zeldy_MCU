
## Custom components

set(COMPONENT_CUSTOM_INFLUXDB "influxdb.h" "influxdb.c")
set(COMPONENT_CUSTOM_ADC "adc.h" "adc.c")

## standard compoenents
set(COMPONENT_SRCS "app_wifi.c"
        ${COMPONENT_CUSTOM_INFLUXDB}
        ${COMPONENT_CUSTOM_ADC}
        )
set(COMPONENT_ADD_INCLUDEDIRS ".")
# Embed the server root certificate into the final binary
#
# (If this was a component, we would set COMPONENT_EMBED_TXTFILES here.)
set(COMPONENT_EMBED_TXTFILES ssl_com_root_cert.pem)

register_component()
