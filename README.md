#gwdata-server

收集无线传感器数据，并对外提供ipv4，ipv6连接。

#使用示例

**ipv4:**

    [~]$ nc 192.168.1.133 51001
    {"device_id":154,"device_type":36,"transfer_type":"zigbee","timestamp":"2000-01-01 10:46:09","device_value":"false"}
    {"device_id":4,"device_type":21,"transfer_type":"zigbee","timestamp":"2000-01-01 10:46:09","device_value":"Ax:326,Ay:-1096,Az:16476,Gx:-48,Gy:-11,Gz:-25"}
    {"device_id":7,"device_type":19,"transfer_type":"zigbee","timestamp":"2000-01-01 10:46:10","device_value":"1780"}
    {"device_id":105,"device_type":25,"transfer_type":"zigbee","timestamp":"2000-01-01 10:46:10","device_value":"3,0"}
    {"device_id":151,"device_type":20,"transfer_type":"zigbee","timestamp":"2000-01-01 10:46:10","device_value":"false"}
   
**ipv6:**
    
    [~]$ nc6 fe80::74ac:6aff:fe71:101%em1 51001
    {"device_id":8,"device_type":29,"transfer_type":"zigbee","timestamp":"2000-01-01 10:44:13","device_value":"1785"}
    {"device_id":10,"device_type":32,"transfer_type":"zigbee","timestamp":"2000-01-01 10:44:13","device_value":"X:-94,Y:-286,Z:-189"}
    {"device_id":151,"device_type":20,"transfer_type":"zigbee","timestamp":"2000-01-01 10:44:13","device_value":"false"}
    {"device_id":7,"device_type":19,"transfer_type":"zigbee","timestamp":"2000-01-01 10:44:14","device_value":"1777"}

   *每包数据以换行符结束,每包数据帧都是一条json格式的字符串。*
 
