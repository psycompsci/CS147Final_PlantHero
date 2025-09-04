/* eslint-disable max-classes-per-file */
/* eslint-disable no-restricted-globals */
/* eslint-disable no-undef */
$(document).ready(() => {
  // if deployed to a site supporting SSL, use wss://
  const protocol = document.location.protocol.startsWith('https') ? 'wss://' : 'ws://';
  const webSocket = new WebSocket(protocol + location.host);

  // A class for holding the last N points of telemetry for a device
  class DeviceData {
    constructor(deviceId) {
      this.deviceId = deviceId;
      this.maxLen = 50;
      this.timeData = new Array(this.maxLen);
      this.temperatureData = new Array(this.maxLen);
      this.humidityData = new Array(this.maxLen);
      this.moistureData = new Array(this.maxLen);
      this.uvData = new Array(this.maxLen);
    }

    addData(time, temperature, humidity, moisture, uvIndex) {
      this.timeData.push(time);
      this.temperatureData.push(temperature);
      this.humidityData.push(humidity || null);
      this.moistureData.push(moisture || null);
      this.uvData.push(uvIndex || null);


      if (this.timeData.length > this.maxLen) {
        this.timeData.shift();
        this.temperatureData.shift();
        this.humidityData.shift();
        this.moistureData.shift();
        this.uvData.shift();
      }
    }
  }

  // All the devices in the list (those that have been sending telemetry)
  class TrackedDevices {
    constructor() {
      this.devices = [];
    }

    // Find a device based on its Id
    findDevice(deviceId) {
      for (let i = 0; i < this.devices.length; ++i) {
        if (this.devices[i].deviceId === deviceId) {
          return this.devices[i];
        }
      }

      return undefined;
    }

    getDevicesCount() {
      return this.devices.length;
    }
  }

  const trackedDevices = new TrackedDevices();

  const temperatureChartConfig = {
    type: 'line',
    data: { labels: [], datasets: [{ label: 'Temperature (°C)', data: [], borderColor: 'rgba(255,204,0,1)', backgroundColor: 'rgba(255,204,0,0.4)', fill: false }] },
    options: { scales: { y: { suggestedMin: 0, suggestedMax: 50, title: { display: true, text: 'Temperature (°C)' } } } }
  };

  const humidityChartConfig = {
    type: 'line',
    data: { labels: [], datasets: [{ label: 'Humidity (%)', data: [], borderColor: 'rgba(24,120,240,1)', backgroundColor: 'rgba(24,120,240,0.4)', fill: false }] },
    options: { scales: { y: { suggestedMin: 0, suggestedMax: 100, title: { display: true, text: 'Humidity (%)' } } } }
  };

  const moistureChartConfig = {
    type: 'line',
    data: { labels: [], datasets: [{ label: 'Moisture', data: [], borderColor: 'rgba(0,255,0,1)', backgroundColor: 'rgba(0,255,0,0.4)', fill: false }] },
    options: { scales: { y: { suggestedMin: 0, suggestedMax: 4095, title: { display: true, text: 'Moisture' } } } }
  };

  const uvChartConfig = {
    type: 'line',
    data: { labels: [], datasets: [{ label: 'UV Index', data: [], borderColor: 'rgba(255,0,255,1)', backgroundColor: 'rgba(255,0,255,0.4)', fill: false }] },
    options: { scales: { y: { suggestedMin: 0, suggestedMax: 12, title: { display: true, text: 'UV Index' } } } }
  };

  const temperatureChart = new Chart(document.getElementById('temperatureChart').getContext('2d'), temperatureChartConfig);
  const humidityChart = new Chart(document.getElementById('humidityChart').getContext('2d'), humidityChartConfig);
  const moistureChart = new Chart(document.getElementById('moistureChart').getContext('2d'), moistureChartConfig);
  const uvChart = new Chart(document.getElementById('uvChart').getContext('2d'), uvChartConfig);

  function updateCharts(device) {
    temperatureChart.data.labels = device.timeData;
    temperatureChart.data.datasets[0].data = device.temperatureData;
    temperatureChart.update();

    humidityChart.data.labels = device.timeData;
    humidityChart.data.datasets[0].data = device.humidityData;
    humidityChart.update();

    moistureChart.data.labels = device.timeData;
    moistureChart.data.datasets[0].data = device.moistureData;
    moistureChart.update();

    uvChart.data.labels = device.timeData;
    uvChart.data.datasets[0].data = device.uvData;
    uvChart.update();
  }

  // Manage a list of devices in the UI, and update which device data the chart is showing
  // based on selection
  let needsAutoSelect = true;
  const deviceCount = document.getElementById('deviceCount');
  const listOfDevices = document.getElementById('listOfDevices');
  function OnSelectionChange() {
    const device = trackedDevices.findDevice(listOfDevices[listOfDevices.selectedIndex].text);
    updateCharts(device);
  }
  listOfDevices.addEventListener('change', OnSelectionChange, false);

  
  // When a web socket message arrives:
  // 1. Unpack it
  // 2. Validate it has date/time and temperature
  // 3. Find or create a cached device to hold the telemetry data
  // 4. Append the telemetry data
  // 5. Update the chart UI
  webSocket.onmessage = function onMessage(message) {
    try {
      const messageData = JSON.parse(message.data);
      console.log(messageData);

      // time and either temperature or humidity are required
      if (!messageData.MessageDate || (!messageData.IotData.temperature && !messageData.IotData.humidity && !messageData.IotData.moisture && !messageData.IotData.uvIndex)) {
        return;
      }

      // find or add device to list of tracked devices
      const existingDeviceData = trackedDevices.findDevice(messageData.DeviceId);

      if (existingDeviceData) {
        existingDeviceData.addData(messageData.MessageDate, messageData.IotData.temperature, messageData.IotData.humidity, messageData.IotData.moisture, messageData.IotData.uvIndex);
      } else {
        const newDeviceData = new DeviceData(messageData.DeviceId);
        trackedDevices.devices.push(newDeviceData);
        const numDevices = trackedDevices.getDevicesCount();
        deviceCount.innerText = numDevices === 1 ? `${numDevices} device` : `${numDevices} devices`;
        newDeviceData.addData(messageData.MessageDate, messageData.IotData.temperature, messageData.IotData.humidity, messageData.IotData.moisture, messageData.IotData.uvIndex);

        // add device to the UI list
        const node = document.createElement('option');
        const nodeText = document.createTextNode(messageData.DeviceId);
        node.appendChild(nodeText);
        listOfDevices.appendChild(node);

        // if this is the first device being discovered, auto-select it
        if (needsAutoSelect) {
          needsAutoSelect = false;
          listOfDevices.selectedIndex = 0;
          OnSelectionChange();
        }
      }

      updateCharts(existingDeviceData || newDeviceData);
    } catch (err) {
      console.error(err);
    }
  };
});


// // /* eslint-disable max-classes-per-file */
// /* eslint-disable no-restricted-globals */
// /* eslint-disable no-undef */
// $(document).ready(() => {
//   // if deployed to a site supporting SSL, use wss://
//   const protocol = document.location.protocol.startsWith('https') ? 'wss://' : 'ws://';
//   const webSocket = new WebSocket(protocol + location.host);

//   // A class for holding the last N points of telemetry for a device
//   class DeviceData {
//     constructor(deviceId) {
//       this.deviceId = deviceId;
//       this.maxLen = 50;
//       this.timeData = new Array(this.maxLen);
//       this.temperatureData = new Array(this.maxLen);
//       this.humidityData = new Array(this.maxLen);
//       this.moistureData = new Array(this.maxLen);
//       this.uvData = new Array(this.maxLen);
//     }

//     addData(time, temperature, humidity, moisture, uvIndex) {
//       this.timeData.push(time);
//       this.temperatureData.push(temperature);
//       this.humidityData.push(humidity || null);
//       this.moistureData.push(moisture || null);
//       this.uvData.push(uvIndex || null);

//       if (this.timeData.length > this.maxLen) {
//         this.timeData.shift();
//         this.temperatureData.shift();
//         this.humidityData.shift();
//         this.moistureData.shift();
//         this.uvData.shift();
//       }
//     }
//   }

//   // All the devices in the list (those that have been sending telemetry)
//   class TrackedDevices {
//     constructor() {
//       this.devices = [];
//     }

//     // Find a device based on its Id
//     findDevice(deviceId) {
//       for (let i = 0; i < this.devices.length; ++i) {
//         if (this.devices[i].deviceId === deviceId) {
//           return this.devices[i];
//         }
//       }

//       return undefined;
//     }

//     getDevicesCount() {
//       return this.devices.length;
//     }
//   }

//   const trackedDevices = new TrackedDevices();

//   // Define the chart axes
//   const chartData = {
//     datasets: [
//       {
//         fill: false,
//         label: 'Temperature',
//         yAxisID: 'Temperature',
//         borderColor: 'rgba(255, 204, 0, 1)',
//         pointBoarderColor: 'rgba(255, 204, 0, 1)',
//         backgroundColor: 'rgba(255, 204, 0, 0.4)',
//         pointHoverBackgroundColor: 'rgba(255, 204, 0, 1)',
//         pointHoverBorderColor: 'rgba(255, 204, 0, 1)',
//         spanGaps: true,
//       },
//       {
//         fill: false,
//         label: 'Humidity',
//         yAxisID: 'Humidity',
//         borderColor: 'rgba(24, 120, 240, 1)',
//         pointBoarderColor: 'rgba(24, 120, 240, 1)',
//         backgroundColor: 'rgba(24, 120, 240, 0.4)',
//         pointHoverBackgroundColor: 'rgba(24, 120, 240, 1)',
//         pointHoverBorderColor: 'rgba(24, 120, 240, 1)',
//         spanGaps: true,
//       },
//       {
//         fill: false,
//         label: 'Moisture',
//         yAxisID: 'Moisture',
//         borderColor: 'rgba(0,255,0,1)',
//         backgroundColor: 'rgba(0,255,0,0.4)',
//         spanGaps: true,
//       },
//       {
//         fill: false,
//         label: 'UV Index',
//         yAxisID: 'UV',
//         borderColor: 'rgba(255,0,255,1)',
//         backgroundColor: 'rgba(255,0,255,0.4)',
//         spanGaps: true,
//       }
//     ]
//   };

//   const chartOptions = {
//     scales: {
//       yAxes: [{
//         id: 'Temperature',
//         type: 'linear',
//         scaleLabel: {
//           labelString: 'Temperature (ºC)',
//           display: true,
//         },
//         position: 'left',
//         ticks: {
//           suggestedMin: 0,
//           suggestedMax: 100,
//           beginAtZero: true
//         }
//       },
//       {
//         id: 'Humidity',
//         type: 'linear',
//         scaleLabel: {
//           labelString: 'Humidity (%)',
//           display: true,
//         },
//         position: 'right',
//         ticks: {
//           suggestedMin: 0,
//           suggestedMax: 100,
//           beginAtZero: true
//         }
//       },
//       {
//         id: 'Moisture',
//         type: 'linear',
//         scaleLabel: {
//           labelString: 'Moisture',
//           display: true,
//         },
//         position: 'right',
//         ticks: {
//           suggestedMin: 0,
//           suggestedMax: 4095,
//           beginAtZero: true
//         }
//       },
//       {
//         id: 'UV',
//         type: 'linear',
//         scaleLabel: {
//           labelString: 'UV Index',
//           display: true,
//         },
//         position: 'right',
//         ticks: {
//           suggestedMin: 0,
//           suggestedMax: 12,
//           beginAtZero: true
//         }
//       }
//     ]
//     }
//   };

//   // Get the context of the canvas element we want to select
//   const ctx = document.getElementById('iotChart').getContext('2d');
//   const myLineChart = new Chart(
//     ctx,
//     {
//       type: 'line',
//       data: chartData,
//       options: chartOptions,
//     });

//   // Manage a list of devices in the UI, and update which device data the chart is showing
//   // based on selection
//   let needsAutoSelect = true;
//   const deviceCount = document.getElementById('deviceCount');
//   const listOfDevices = document.getElementById('listOfDevices');
//   function OnSelectionChange() {
//     const device = trackedDevices.findDevice(listOfDevices[listOfDevices.selectedIndex].text);
//     chartData.labels = device.timeData;
//     chartData.datasets[0].data = device.temperatureData;
//     chartData.datasets[1].data = device.humidityData;
//     chartData.datasets[2].data = device.moistureData;
//     chartData.datasets[3].data = device.uvData;
//     myLineChart.update();
//   }
//   listOfDevices.addEventListener('change', OnSelectionChange, false);

//   // When a web socket message arrives:
//   // 1. Unpack it
//   // 2. Validate it has date/time and temperature
//   // 3. Find or create a cached device to hold the telemetry data
//   // 4. Append the telemetry data
//   // 5. Update the chart UI
//   webSocket.onmessage = function onMessage(message) {
//     try {
//       const messageData = JSON.parse(message.data);
//       console.log(messageData);

//       // time and either temperature or humidity are required
//       if (!messageData.MessageDate || (!messageData.IotData.temperature && !messageData.IotData.humidity)) {
//         return;
//       }

//       // find or add device to list of tracked devices
//       const existingDeviceData = trackedDevices.findDevice(messageData.DeviceId);

//       if (existingDeviceData) {
//         existingDeviceData.addData(messageData.MessageDate, messageData.IotData.temperature, messageData.IotData.humidity, messageData.IotData.moisture, messageData.IotData.uvIndex);
//       } else {
//         const newDeviceData = new DeviceData(messageData.DeviceId);
//         trackedDevices.devices.push(newDeviceData);
//         const numDevices = trackedDevices.getDevicesCount();
//         deviceCount.innerText = numDevices === 1 ? `${numDevices} device` : `${numDevices} devices`;
//         newDeviceData.addData(messageData.MessageDate, messageData.IotData.temperature, messageData.IotData.humidity, messageData.IotData.moisture, messageData.IotData.uvIndex);

//         // add device to the UI list
//         const node = document.createElement('option');
//         const nodeText = document.createTextNode(messageData.DeviceId);
//         node.appendChild(nodeText);
//         listOfDevices.appendChild(node);

//         // if this is the first device being discovered, auto-select it
//         if (needsAutoSelect) {
//           needsAutoSelect = false;
//           listOfDevices.selectedIndex = 0;
//           OnSelectionChange();
//         }
//       }

//       myLineChart.update();
//     } catch (err) {
//       console.error(err);
//     }
//   };
// });
