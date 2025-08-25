@echo off
C:\Users\mail\.local\bin\protoc --c_out=components/aircom_proto --proto_path=. AirCom.proto
C:\Users\mail\AppData\Local\Programs\Python\Python310\Scripts\pio run -e xiao_esp32s3