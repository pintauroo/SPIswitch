webtalk_init -webtalk_dir C:\\Users\\Andrea\\SPIswitch\\SPIswitch.sdk\\webtalk
webtalk_register_client -client project
webtalk_add_data -client project -key date_generated -value "2021-10-25 09:59:23" -context "software_version_and_target_device"
webtalk_add_data -client project -key product_version -value "SDK v2018.3" -context "software_version_and_target_device"
webtalk_add_data -client project -key build_version -value "2018.3" -context "software_version_and_target_device"
webtalk_add_data -client project -key os_platform -value "amd64" -context "software_version_and_target_device"
webtalk_add_data -client project -key registration_id -value "212308080_0_0_265" -context "software_version_and_target_device"
webtalk_add_data -client project -key tool_flow -value "SDK" -context "software_version_and_target_device"
webtalk_add_data -client project -key beta -value "false" -context "software_version_and_target_device"
webtalk_add_data -client project -key route_design -value "NA" -context "software_version_and_target_device"
webtalk_add_data -client project -key target_family -value "NA" -context "software_version_and_target_device"
webtalk_add_data -client project -key target_device -value "NA" -context "software_version_and_target_device"
webtalk_add_data -client project -key target_package -value "NA" -context "software_version_and_target_device"
webtalk_add_data -client project -key target_speed -value "NA" -context "software_version_and_target_device"
webtalk_add_data -client project -key random_id -value "6o5qcsduphostadn4ms53h0dks" -context "software_version_and_target_device"
webtalk_add_data -client project -key project_id -value "2018.3_6" -context "software_version_and_target_device"
webtalk_add_data -client project -key project_iteration -value "6" -context "software_version_and_target_device"
webtalk_add_data -client project -key os_name -value "Microsoft Windows 8 or later , 64-bit" -context "user_environment"
webtalk_add_data -client project -key os_release -value "major release  (build 9200)" -context "user_environment"
webtalk_add_data -client project -key cpu_name -value "Intel(R) Core(TM) i7-6700HQ CPU @ 2.60GHz" -context "user_environment"
webtalk_add_data -client project -key cpu_speed -value "2592 MHz" -context "user_environment"
webtalk_add_data -client project -key total_processors -value "1" -context "user_environment"
webtalk_add_data -client project -key system_ram -value "17.034 GB" -context "user_environment"
webtalk_register_client -client sdk
webtalk_add_data -client sdk -key uid -value "1635010002504" -context "sdk\\\\bsp/1635010002504"
webtalk_add_data -client sdk -key hwid -value "1635008351083" -context "sdk\\\\bsp/1635010002504"
webtalk_add_data -client sdk -key os -value "standalone" -context "sdk\\\\bsp/1635010002504"
webtalk_add_data -client sdk -key apptemplate -value "hello_world" -context "sdk\\\\bsp/1635010002504"
webtalk_add_data -client sdk -key RecordType -value "BSPCreation" -context "sdk\\\\bsp/1635010002504"
webtalk_add_data -client sdk -key uid -value "1635010004290" -context "sdk\\\\application/1635010004290"
webtalk_add_data -client sdk -key hwid -value "1635008351083" -context "sdk\\\\application/1635010004290"
webtalk_add_data -client sdk -key bspid -value "1635010002504" -context "sdk\\\\application/1635010004290"
webtalk_add_data -client sdk -key newbsp -value "true" -context "sdk\\\\application/1635010004290"
webtalk_add_data -client sdk -key os -value "standalone" -context "sdk\\\\application/1635010004290"
webtalk_add_data -client sdk -key apptemplate -value "hello_world" -context "sdk\\\\application/1635010004290"
webtalk_add_data -client sdk -key RecordType -value "APPCreation" -context "sdk\\\\application/1635010004290"
webtalk_add_data -client sdk -key LangUsed -value "C" -context "sdk\\\\application/1635010004290"
webtalk_add_data -client sdk -key Procused -value "microblaze" -context "sdk\\\\application/1635010004290"
webtalk_add_data -client sdk -key projSize -value "285.28515625" -context "sdk\\\\application/1635010004290"
webtalk_add_data -client sdk -key uid -value "NA" -context "sdk\\\\bsp/1635173963787"
webtalk_add_data -client sdk -key RecordType -value "ToolUsage" -context "sdk\\\\bsp/1635173963787"
webtalk_add_data -client sdk -key BootgenCount -value "0" -context "sdk\\\\bsp/1635173963787"
webtalk_add_data -client sdk -key DebugCount -value "82" -context "sdk\\\\bsp/1635173963787"
webtalk_add_data -client sdk -key PerfCount -value "0" -context "sdk\\\\bsp/1635173963787"
webtalk_add_data -client sdk -key FlashCount -value "0" -context "sdk\\\\bsp/1635173963787"
webtalk_add_data -client sdk -key CrossTriggCount -value "0" -context "sdk\\\\bsp/1635173963787"
webtalk_add_data -client sdk -key QemuDebugCount -value "0" -context "sdk\\\\bsp/1635173963787"
webtalk_transmit -clientid 1890255786 -regid "212308080_0_0_265" -xml C:\\Users\\Andrea\\SPIswitch\\SPIswitch.sdk\\webtalk\\usage_statistics_ext_sdk.xml -html C:\\Users\\Andrea\\SPIswitch\\SPIswitch.sdk\\webtalk\\usage_statistics_ext_sdk.html -wdm C:\\Users\\Andrea\\SPIswitch\\SPIswitch.sdk\\webtalk\\sdk_webtalk.wdm -intro "<H3>SDK Usage Report</H3><BR>"
webtalk_terminate
