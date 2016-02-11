<?php

/**
 * @file
 * Defines the inline entity form controller for Notes.
 */

class HACoreNoteInlineEntityFormController extends EntityInlineEntityFormController {

  /**
  * Overrides EntityInlineEntityFormController::defaultLabels().
   */
  public function defaultLabels() {
    $labels = array(
      'singular' => t('note'),
      'plural' => t('notes'),
    );
    return $labels;
  }

  /**
   * Overrides EntityInlineEntityFormController::entityForm().
   */
  public function entityForm($entity_form, &$form_state) {
    $note = $entity_form['#entity'];
    $extra_fields = field_info_extra_fields('ha_core_note', 'ha_core_note', 'form');

    $defaults = array(
      'id' => '',
      'body' => NULL,
      'date' => NULL,
    );
    foreach ($defaults as $key => $value) {
      if (!isset($note->$key)) {
        $note->$key = $value;
      }
    }

    $entity_form['body'] = array(
      '#type' => 'textarea',
      '#title' => t('Body'),
      '#default_value' => $note->body,
      '#required' => TRUE,
      '#weight' => $extra_fields['body']['weight'],
    );

    $langcode = entity_language('ha_core_note', $note);
    field_attach_form('ha_core_note', $note, $entity_form, $form_state, $langcode);

    return $entity_form;
  }
}

