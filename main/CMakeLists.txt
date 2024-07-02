set(COMPONENT_SRCS "device_shadow.c"
                   "aws_custom_utils.c" 
                   "wifi-connect.c" 
                   "output_driver.c" 
                   "sub_pub_ota.c"
                   "ota.c"
                   "main.c")

set(COMPONENT_ADD_INCLUDEDIRS "")

register_component()

target_add_binary_data(${COMPONENT_TARGET} "cloud_certs/server.cert" TEXT)
target_add_binary_data(${COMPONENT_TARGET} "cloud_certs/device.cert" TEXT)
target_add_binary_data(${COMPONENT_TARGET} "cloud_certs/device.key" TEXT)
target_add_binary_data(${COMPONENT_TARGET} "cloud_certs/deviceid.txt" TEXT)
target_add_binary_data(${COMPONENT_TARGET} "cloud_certs/endpoint.txt" TEXT)
target_add_binary_data(${COMPONENT_TARGET} "cloud_certs/github_server.cert" TEXT)