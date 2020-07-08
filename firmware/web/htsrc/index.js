'use strict';

const root = document.body;

const doc_contacts_ui = document.getElementById("contacts-ui");
const doc_environmentals_ui = document.getElementById("environmentals-ui");

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

m.mount(doc_contacts_ui, contacts_ui);

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

m.mount(doc_environmentals_ui, environmentals_ui);

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

status_update();
setInterval(status_update, 1000);