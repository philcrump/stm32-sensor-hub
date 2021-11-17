'use strict';

var doc_contacts_ui;
var doc_environmentals_ui;

var updated = 0;

var contacts_obj = [];

var environmentals_obj = [];

var contacts_ui = {
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

var environmentals_ui = {
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
          m("td", `${environmental_obj.temperature}°C`),
          m("td", `${environmental_obj.humidity}% rH`),
        ]);
      })
    );
  }
}

function status_update()
{
  m.request({
    method: "GET",
    url: "/api/status"
  })
  .then(function(result) {
    contacts_obj = result.contacts;
    environmentals_obj = result.environmentals;
    updated = Date.now();
  });
  /* Manual redraw for 'updated' */
  m.redraw();
}

window.onload = function()
{
  doc_contacts_ui = document.getElementById("contacts-ui");
  doc_environmentals_ui = document.getElementById("environmentals-ui");

  m.mount(doc_contacts_ui, contacts_ui);
  m.mount(doc_environmentals_ui, environmentals_ui);

  status_update();
  setInterval(status_update, 1000);
}