<?php

class HACoreNoteController extends EntityAPIController {

  /**
   * Override the entity create method.
   */
  public function create(array $values = array()) {
    global $user;

    // Add author's uid to the note
    $values['uid'] = $user->uid;

    return parent::create($values);
  }

  /**
   * Override the entity save method.
   */
  public function save($entity, DatabaseTransaction $transaction = NULL) {
    global $user;

    // If user is a technician, make the note private automatically
    if (array_search('technician', $user->roles) !== false) {
      $entity->field_only_visible_to_mgmt['und']['0']['value'] = 1;
    }

    // Set date to current time
    $entity->date = REQUEST_TIME;

    parent::save($entity, $transaction);

    return $entity->id;
  }

  /**
   * Override the entity buildContent method.
   */
  public function buildContent($entity, $view_mode = 'full', $langcode = NULL, $content = array()) {
    $wrapper = entity_metadata_wrapper('ha_core_note', $entity);
    $username = $wrapper->uid->name->value(array('sanitize' => TRUE));
    $date = date(DATE_COOKIE, check_plain($entity->date));

    $content['body'] = array(
      '#type' => 'markup',
      '#markup' => check_plain($entity->body),
      '#prefix' => '<div>',
      '#suffix' => '</div>',
    );
    $content['author'] = array(
      '#type' => 'markup',
      '#markup' => t('Created by: !author', array('!author' => $username)),
      '#suffix' => '<br>',
    );
    $content['date'] = array(
      '#type' => 'markup',
      '#markup' => t('Last Modified Date: !date', array('!date' => $date)),
      '#prefix' => '<span>',
      '#suffix' => '</span>',
    );

    return parent::buildContent($entity, $view_mode, $langcode, $content);
  }
}
