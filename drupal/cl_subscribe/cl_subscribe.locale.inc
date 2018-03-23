<?php

/**
 * Implements hook_countries_alter().
 */
function cl_subscribe_countries_alter(&$countries) {
  // Move US to the top of the countries list
  $countries = array('US' => $countries['US']) + $countries;
}

/**
 * Get list of US states.
 *
 * @return
 *   An array of all state abbrev => state name pairs.
 */
function _cl_subscribe_states_get_list() {
  return array(
    '' => t('Not Listed/Not Applicable'),
    'AK' => t('Alaska'),
    'AL' => t('Alabama'),
    'AR' => t('Arkansas'),
    'AS' => t('American Samoa'),
    'AZ' => t('Arizona'),
    'CA' => t('California'),
    'CO' => t('Colorado'),
    'CT' => t('Connecticut'),
    'DC' => t('D.C.'),
    'DE' => t('Delaware'),
    'FL' => t('Florida'),
    'FM' => t('Micronesia'),
    'GA' => t('Georgia'),
    'GU' => t('Guam'),
    'HI' => t('Hawaii'),
    'IA' => t('Iowa'),
    'ID' => t('Idaho'),
    'IL' => t('Illinois'),
    'IN' => t('Indiana'),
    'KS' => t('Kansas'),
    'KY' => t('Kentucky'),
    'LA' => t('Louisiana'),
    'MA' => t('Massachusetts'),
    'MD' => t('Maryland'),
    'ME' => t('Maine'),
    'MH' => t('Marshall Islands'),
    'MI' => t('Michigan'),
    'MN' => t('Minnesota'),
    'MO' => t('Missouri'),
    'MP' => t('Marianas'),
    'MS' => t('Mississippi'),
    'MT' => t('Montana'),
    'NC' => t('North Carolina'),
    'ND' => t('North Dakota'),
    'NE' => t('Nebraska'),
    'NH' => t('New Hampshire'),
    'NJ' => t('New Jersey'),
    'NM' => t('New Mexico'),
    'NV' => t('Nevada'),
    'NY' => t('New York'),
    'OH' => t('Ohio'),
    'OK' => t('Oklahoma'),
    'OR' => t('Oregon'),
    'PA' => t('Pennsylvania'),
    'PR' => t('Puerto Rico'),
    'PW' => t('Palau'),
    'RI' => t('Rhode Island'),
    'SC' => t('South Carolina'),
    'SD' => t('South Dakota'),
    'TN' => t('Tennessee'),
    'TX' => t('Texas'),
    'UT' => t('Utah'),
    'VA' => t('Virginia'),
    'VI' => t('Virgin Islands'),
    'VT' => t('Vermont'),
    'WA' => t('Washington'),
    'WI' => t('Wisconsin'),
    'WV' => t('West Virginia'),
    'WY' => t('Wyoming'),
    'AA' => t('Military Americas'),
    'AE' => t('Military Europe/ME/Canada'),
    'AP' => t('Military Pacific'),
  );
}

/**
 * Get list of Canadian provinces.
 *
 * @return
 *   An array of all province abbrev => province name pairs.
 */
function _cl_subscribe_provinces_get_list() {
  return array(
    '' => t('Not Listed/Not Applicable'),
    'AB' => t('Alberta'),
    'MB' => t('Manitoba'),
    'BC' => t('British Columbia'),
    'NB' => t('New Brunswick'),
    'NF' => t('Newfoundland and Labrador'),
    'NS' => t('Nova Scotia'),
    'NT' => t('Northwest Territories'),
    'NU' => t('Nunavut'),
    'ON' => t('Ontario'),
    'PE' => t('Prince Edward Island'),
    'QC' => t('Quebec'),
    'SK' => t('Sakatchewan'),
    'YT' => t('Yukon Territory'),
  );
}
