'use strict';

let doc_co2_ui;
let co2_graph_6h = null;

let doc_contacts_ui;
let doc_environmentals_ui;
let doc_mcu_info_ui;

let updated = 0;

let mcu_info_obj = null;
let co2_data = null;
let co2_history = null;
let contacts_obj = [];
let environmentals_obj = [];

let co2_ui = {
  view: function()
  {
    if(co2_data == null)
    {
      return;
    }

    const co2_badgeclass = function(co2_ppm_value)
    {
      if(co2_ppm_value > 1500)
      {
        return 'bg-danger';
      }
      else if(co2_ppm_value > 1200)
      {
        return 'bg-warning';
      }
      return 'bg-success';
    }

    return m("table", {id: 'co2-ui-table'},
      [
        m("tr", [
          m("td", {'class': 'fw-bold'}, "CO₂"),
          m("td",
            m("span",
              {'class': `fs-4 badge ${co2_badgeclass(co2_data.avg_co2_ppm)}`},
              `${decimalFix(co2_data.avg_co2_ppm,1)} ppm`
            )
          )
        ]),
        m("tr", [
          m("td", {'class': 'fw-bold'}, "Temperature"),
          m("td",
            m("span",
              {'class': `fs-4 badge bg-light text-dark`},
              `${decimalFix(co2_data.avg_temperature_c,1)}°C`
            )
          )
        ]),
        m("tr", [
          m("td", {'class': 'fw-bold'}, "Humidity"),
          m("td",
            m("span",
              {'class': `fs-4 badge bg-light text-dark`},
              `${decimalFix(co2_data.avg_humidity_rh,1)}%`
            )
          )
        ])
      ]
    );
  }
}

let contacts_ui = {
  view: function()
  {
    if(contacts_obj.length == 0)
    {
      return m("i", "No contacts configured.");
    }

    return m("table", {id: 'contacts-ui-table'},
      m("tr", [
        m("th", "Channel"),
        m("th", "State"),
      ]),
      contacts_obj.map(function(contact_obj) {
        return m("tr", [
          m("td", contact_obj.name),
          m("td", contact_obj.value == 0 ? "Open" : "Closed"),
        ]);
      })
    );
  }
}

let environmentals_ui = {
  view: function()
  {
    if(environmentals_obj.length == 0)
    {
      return m("i", "No environmentals configured.");
    }

    return m("table", {id: 'environmentals-ui-table'},
      m("tr", [
        m("th", "Channel"),
        m("th", "Temperature"),
        m("th", "Humidity"),
      ]),
      environmentals_obj.map(function(environmental_obj) {
        return m("tr", [
          m("td", environmental_obj.name),
          m("td", `${decimalFix(environmental_obj.temperature,1)}°C`),
          m("td", `${decimalFix(environmental_obj.humidity,1)}% rH`),
        ]);
      })
    );
  }
}

let mcu_info_ui = {
  view: function()
  {
    if(mcu_info_obj == null)
    {
      return;
    }

    return m("table", {id: 'mcu-info-ui-table'},
      [
        m("tr", [
          m("td", {'class': 'fw-bold'}, "Firmware Name"),
          m("td", `${mcu_info_obj.firmware_name}`),
        ]),
        m("tr", [
          m("td", {'class': 'fw-bold'}, "Firmware Version"),
          m("td", `${mcu_info_obj.firmware_version}`),
        ]),
        m("tr", [
          m("td", {'class': 'fw-bold'}, "MCU Temperature"),
          m("td", `${decimalFix(mcu_info_obj.temperature_degrees,1)} °C`),
        ]),
        m("tr", [
          m("td", {'class': 'fw-bold'}, "VBat Voltage"),
          m("td", `${decimalFix(mcu_info_obj.vbat_volts,2)} V`),
        ])
      ]
    );
  }
}

function contacts_update()
{
  m.request({
    method: "GET",
    url: "/api/contacts"
  })
  .then(function(result) {
    contacts_obj = result.contacts;
    updated = Date.now();
  });
  /* Manual redraw for 'updated' */
  m.redraw();
}

function environmentals_update()
{
  m.request({
    method: "GET",
    url: "/api/environmentals"
  })
  .then(function(result) {
    environmentals_obj = result.environmentals;
    updated = Date.now();
  });
  /* Manual redraw for 'updated' */
  m.redraw();
}

function scd30_update()
{
  m.request({
    method: "GET",
    url: "/api/scd30"
  })
  .then(function(result) {
    co2_data = result;
  });
  /* Manual redraw for 'updated' */
  m.redraw();
}

function scd30_history_update()
{
  m.request({
    method: "GET",
    url: "/api/scd30_history_co2"
  })
  .then(function(result)
  {
    const dateNow = new Date();

    const interval_ms = result.interval_seconds * 1000;

    co2_history = result.data.map((value, index) => [
      new Date(dateNow.getTime() - (interval_ms * (result.length - index))),
      value
    ]);

    if(co2_history != null && co2_history.length > 0)
    {
      if(co2_graph_6h == null)
      {
        co2_graph_6h = new Dygraph(
          document.getElementById("co2-graph"),
          co2_history,
          {
            drawPoints: true,
            legend: 'always',
            labels: ['Time', 'CO₂'],
            valueRange: [0, 2001],
            showLabelsOnHighlight: false,
            labelsUTC: true,
            title: 'CO₂ Concentration',
            ylabel: 'CO₂ (ppm)',
            xlabel: 'Time',
            color: 'Blue',
            underlayCallback: function(canvas, area, g)
            {
              const min_data_x = g.getValue(0,0);
              const max_data_x = g.getValue(g.numRows()-1,0);

              /* Yellow - Warning */
              let bottom_left = g.toDomCoords(min_data_x, 1000);
              let top_right = g.toDomCoords(max_data_x, 2001);

              let grd = canvas.createLinearGradient(0, 0, 0, bottom_left[1] - top_right[1]);
              /* Red - Critical */
              grd.addColorStop(0.0, "rgba(255, 0, 0, 0.3)");
              /* Amber - Warning */
              grd.addColorStop(0.5, "rgba(255, 128, 0, 0.3)");
              /* White */
              grd.addColorStop(1.0, "rgba(255, 128, 0, 0.0)");

              canvas.fillStyle = grd;
              canvas.fillRect(bottom_left[0], top_right[1] - area.y, top_right[0] - bottom_left[0], bottom_left[1] - top_right[1]);
            }
          }
        );
      }
      else
      {
        co2_graph_6h.updateOptions(
        {
          'file': co2_history
        }
      );
      }
    }
    else
    {
      document.getElementById("co2-graph").innerHTML = '<h4 class="align-middle">Waiting 60s for Data..</h4>';
    }
  });
}

function mcu_info_update()
{
  m.request({
    method: "GET",
    url: "/api/mcu_info"
  })
  .then(function(result) {
    mcu_info_obj = result;
    updated = Date.now();
  });
  /* Manual redraw for 'updated' */
  m.redraw();
}

window.onload = function()
{
  doc_mcu_info_ui = document.getElementById("mcu-info-ui");
  doc_co2_ui = document.getElementById("co2-ui");
  doc_contacts_ui = document.getElementById("contacts-ui");
  doc_environmentals_ui = document.getElementById("environmentals-ui");

  m.mount(doc_mcu_info_ui, mcu_info_ui);
  m.mount(doc_co2_ui, co2_ui);
  m.mount(doc_contacts_ui, contacts_ui);
  m.mount(doc_environmentals_ui, environmentals_ui);

  mcu_info_update();
  setInterval(mcu_info_update, 1000);

  contacts_update();
  setInterval(contacts_update, 1000);

  environmentals_update();
  setInterval(environmentals_update, 1000);

  scd30_update();
  setInterval(scd30_update, 2*1000);

  scd30_history_update();
  setInterval(scd30_history_update, 10*1000);
}


function decimalFix(num, decimals, leading=0, leadingChar='0')
{
  const t = Math.pow(10, decimals);
  return ((Math.round((num * t) + (decimals>0?1:0)*(Math.sign(num) * (10 / Math.pow(100, decimals)))) / t).toFixed(decimals)).toString().padStart(leading+1+decimals, leadingChar);
}