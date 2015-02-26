<?php

/**
 * Implements hook_views_data().
 */
function lcs_views_area_commerce_base_price_views_data() {
  $data['views']['line_item_summary_base_prices'] = array(
    'title' => t('Line item summary with base prices'),
    'help' => t('Get the prices of the line items in base price format'),
    'area' => array(
      'handler' => 'lcs_views_area_commerce_base_price_handler_area_line_items_base_price'
    ),
  );
}