# Code Samples
Samples of my work from jobs and classes.
## c
- These files are from a college C course in 2012.
  - The .c files each contain the algorithm described at the top of the file.
  - The BankProject directory contains a client/server banking application made with a class partner. There is a PDF inside explaining it in more detail.

## drupal
- ha_core is a custom module to setup the HA Lighting website and provide it with a Note custom entity type.
  - HA Lighting is a client site I work on for my part-time Drupal job.
  - All code related to the Note custom entity type is my contribution, along with the initial creation of the module, install functions, and most hooks.
- cl_subscribe is a custom module to provide a newsletter subscription form, and an admin settings page for customizing the form's header and footer.
  - The form accepts an address, and autoupdates the "State/Province" dropdown depending on the selected "Country". Certain fields are also hidden depending on the country and are required when visible but not required when hidden. The combination of those two features presented an interesting challenge.
  - I contributed all code in cl_subscribe.module and cl_subscribe.locale.inc.
- Contributed a patch for the Google Store Locator module. Created two versions of the patch for different versions of the module. They can be seen [here](https://www.drupal.org/node/2640696).
