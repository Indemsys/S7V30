      var device;
      var parameters;
      var productName;
      var HW_ver;
      var SW_ver;
      var CompDate;
      var CompTime;
      var CPU_ID;
      let Fields_filler;

	  
      function Open_menu()
      {
        document.getElementById("Main_menu").style.width = "250px";
      }

      function closeMenu()
      {
        document.getElementById("Main_menu").style.width = "0";
      }

      function insertSpaceEveryEightChars(input)
      {
        return input.replace(/(.{8})(?=.)/g, '$1 ');
      }


      function ExtractDeviceData(data)
      {
        productName = data.Product_name;
        HW_ver      = data.HW_Ver;
        SW_ver      = data.SW_Ver;
        CompDate    = data.CompDate;
        CompTime    = data.CompTime;
        CPU_ID      = data.CPU_ID;
        //document.getElementById('header_text').innerText = productName;
        document.getElementById('footer_text').innerText = insertSpaceEveryEightChars(CPU_ID);
		if (typeof Fields_filler === "function") {Fields_filler();}


	  }

      async function LoadDeviceData()
      {
        try
        {
          const response = await fetch('post_device_info', {method: 'POST'});
          const data = await response.json();

          localStorage.setItem('deviceData', JSON.stringify(data));
          ExtractDeviceData(data);

        } catch (error)
        {
          console.error('Error loading JSON:', error);
        }
      }

      function Check_And_Load_Data()
      {
        const data = localStorage.getItem('deviceData');
        if (data == null)
        {
          const data = LoadDeviceData();
        } else
        {
          ExtractDeviceData(data);
        }
      }
    
      async function Reset_device()
      {
        try
        {
          const response = await fetch('reset', {method: 'POST'});
          if (!response.ok)
          {
            throw new Error('Network response was not ok ' + response.statusText);
          }
        } catch (error)
        {
          alert('There has been a problem with fetch operation:', error);
        }
        closeMenu();
        alert('Connection with the device will be lost now. Reconnect and reload the page after the device is ready.');
      }