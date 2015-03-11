<?php

/**
 * Defines a line item summary area handler so the summary can be plugged into
 *   the View itself.
 */
class lcs_views_area_commerce_base_price_handler_area_line_items_base_price extends views_handler_area {

  function option_definition() {
    $options = parent::option_definition();

    // Define an option to control the links displayed in the summary.
    $options['links'] = array('default' => array());

    foreach (commerce_line_item_summary_links() as $name => $link) {
      $options['links']['default'][$name] = 0;
    }

    // Define an option to control the info displayed in the summary.
    $options['info'] = array('default' => array(
      'quantity' => 1,
      'total' => 1,
      'total_delivery' => 0,
    ));

    // Define an option to control the profit displayed in the summary.
    $options['profit'] = array('default' => array(
      'amount' => 0,
      'percentage' => 0,
    ));

    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    // Add checkboxes for the summary links if any are available.
    $options = array();

    foreach (commerce_line_item_summary_links() as $name => $link) {
      $options[$name] = $link['title'];
    }

    if (!empty($options)) {
      $form['links'] = array(
        '#type' => 'checkboxes',
        '#title' => t('Links'),
        '#description' => t('Select the links you want to appear beneath the summary.'),
        '#options' => $options,
        '#default_value' => $this->options['links'],
      );
    }

    $form['info'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Info'),
      '#description' => t('Select what info you want displayed in the summary.'),
      '#options' => array('quantity' => t('Item quantity'), 'total' => t('Total customer'), 'total_delivery' => t('Total delivery')),
      '#default_value' => $this->options['info'],
    );

    $form['profit'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Profit'),
      '#description' => t('Select what profit info you want displayed in the summary.'),
      '#options' => array('amount' => t('Profit amount'), 'percentage' => t('Profit percentage')),
      '#default_value' => $this->options['profit'],
    );
  }

  /**
   * Get a value used for rendering.
   *
   * @param $values
   *   An object containing all retrieved values.
   * @param $field
   *   Optional name of the field where the value is stored.
   */
  function get_value($values, $field = NULL) {
    // Find the alias for the line_item_id field.
    $field_alias = '';

    foreach ($this->view->query->fields as $key => $value) {
      if ($value['field'] == 'line_item_id') {
        $field_alias = $value['alias'];
      }
    }

    if (isset($values->{$field_alias})) {
      return $values->{$field_alias};
    }
  }

  function render($empty = FALSE) {
    if (!$empty || !empty($this->options['empty'])) {
      // Build an array of line item IDs from the View results that we will load
      // and use for calculating totals.
      $line_item_ids = array();

      foreach ($this->view->result as $result) {
        $line_item_id = $this->get_value($result);
        if ($line_item_id) {
          $line_item_ids[] = $line_item_id;
        }
      }

      $line_items = commerce_line_item_load_multiple($line_item_ids);

      // Add total information and the line item summary links.
      $quantity = commerce_line_items_quantity($line_items);
      $total = lcs_views_area_commerce_base_price_line_items_total($line_items);
      $total_customer = commerce_line_items_total($line_items);
      $currency = commerce_currency_load($total['currency_code']);

      $total_profit_amount = $total_customer['amount'] - $total['amount'];
      // Calculate profit over deliverer price
      $total_profit_percentage = ($total_profit_amount / $total['amount']) * 100;
      $links = array();

      foreach (commerce_line_item_summary_links() as $name => $link) {
        if ($this->options['links'][$name] === $name && $link['access']) {
          $links[str_replace('_', '-', 'line-item-summary-' . $name)] = $link;
        }
      }

      // Build the variables array to send to the template.
      $variables = array(
        'view' => $this->view,
        'links' => theme('links', array('links' => $links, 'attributes' => array('class' => array('links', 'inline')))),
      );
      if ($this->options['info']['quantity']) {
        $variables = array(
            'quantity_raw' => $quantity,
            'quantity_label' => format_plural($quantity, 'item', 'items', array(), array('context' => 'product count on a Commerce order')),
            'quantity' => format_plural($quantity, '1 item', '@count items', array(), array('context' => 'product count on a Commerce order')),
          ) + $variables;
      }
      if ($this->options['info']['total']) {
        $variables = array(
            'total_raw' => number_format(commerce_currency_round($total_customer['amount'], $currency), $currency['decimals']),
            'total_label' => t('Total customer:'),
            'total' => commerce_currency_format($total_customer['amount'], $total_customer['currency_code'], $this->view),
          ) + $variables;
      }
      if ($this->options['info']['total_delivery']) {
        $variables = array(
            'totaldelivery_raw' => number_format(commerce_currency_round($total['amount'], $currency), $currency['decimals']),
            'totaldelivery_label' => t('Total deliverer:'),
            'totaldelivery' => commerce_currency_format($total['amount'], $total['currency_code'], $this->view),
          ) + $variables;
      }
      if ($this->options['profit']['amount']) {
        $variables = array(
            'totalprofitamount_raw' => number_format(commerce_currency_round($total_profit_amount, $currency), $currency['decimals']),
            'totalprofitamount_label' => t('Total profit amount:'),
            'totalprofitamount' => commerce_currency_format($total_profit_amount, $total_customer['currency_code'], $this->view),
          ) + $variables;
      }
      if ($this->options['profit']['percentage']) {
        $variables = array(
            'totalprofitpercentage_raw' => number_format($total_profit_percentage,2) . '%',
            'totalprofitpercentage_label' => t('Total profit percentage:'),
            'totalprofitpercentage' => number_format($total_profit_percentage,2) . '%',
          ) + $variables;
      }

      return theme('commerce_line_item_summary', $variables);
    }

    return '';
  }
}
