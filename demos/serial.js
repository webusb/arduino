var serial = {};
var interfaceNumber=2;		// original interface number of WebUSB Arduino demo
var endpointIn=5;			// original in endpoint ID of WebUSB Arduino demo
var endpointOut=4;			// original out endpoint ID of WebUSB Arduino demo

(function() {
  'use strict';

  serial.getPorts = function() {
    return navigator.usb.getDevices().then(devices => {
      return devices.map(device => new serial.Port(device));
    });
  };

  serial.requestPort = function() {
    const filters = [
      { 'vendorId': 0x2341, 'productId': 0x8036 },
      { 'vendorId': 0x2341, 'productId': 0x8037 },
      { 'vendorId': 0x2341, 'productId': 0x804d },
      { 'vendorId': 0x2341, 'productId': 0x804e },
      { 'vendorId': 0x2341, 'productId': 0x804f },
      { 'vendorId': 0x2341, 'productId': 0x8050 },
    ];
    return navigator.usb.requestDevice({ 'filters': filters }).then(
      device => new serial.Port(device)
    );
  }

  serial.Port = function(device) {
    this.device_ = device;
  };

  serial.Port.prototype.connect = function() {
    let readLoop = () => {
      this.device_.transferIn(endpointIn, 64).then(result => {
        this.onReceive(result.data);
        readLoop();
      }, error => {
        this.onReceiveError(error);
      });
    };

    return this.device_.open()
        .then(() => {
          if (this.device_.configuration === null) {
            return this.device_.selectConfiguration(1);
          }
        })
		.then(()=> {
			this.device_.configuration.interfaces.forEach(function(element) {
				var found=false;
				console.log(element.interfaceNumber);
				
				element.alternates.forEach(function(elementalt) {
					if (elementalt.interfaceClass==0xff) {
						console.log("Found!");
						console.log("interfaceClass=%s",elementalt.interfaceClass);
						found=true;
						interfaceNumber=element.interfaceNumber;
						
						elementalt.endpoints.forEach(function(elementendpoint) {
							if (elementendpoint.direction=="out") {
								endpointOut=elementendpoint.endpointNumber;
								console.log("out endpoint ID=%s", endpointOut);
							}
							if (elementendpoint.direction=="in") {
								endpointIn=elementendpoint.endpointNumber;
								console.log("in endpoint ID=%s", endpointIn);
							}
						})
					}
				})
			})
		})
        .then(() => this.device_.claimInterface(interfaceNumber))
        .then(() => this.device_.selectAlternateInterface(interfaceNumber, 0))
        .then(() => this.device_.controlTransferOut({
            'requestType': 'class',
            'recipient': 'interface',
            'request': 0x22,
            'value': 0x01,
            'index': interfaceNumber}))
        .then(() => {
          readLoop();
        });
  };

  serial.Port.prototype.disconnect = function() {
    return this.device_.controlTransferOut({
            'requestType': 'class',
            'recipient': 'interface',
            'request': 0x22,
            'value': 0x00,
            'index': interfaceNumber})
        .then(() => this.device_.close());
  };

  serial.Port.prototype.send = function(data) {
    return this.device_.transferOut(endpointOut, data);
  };
})();
