import { Component } from '@angular/core';
import { Router } from '@angular/router';

import { PrometheusAlertService } from '~/app/shared/services/prometheus-alert.service';

@Component({
  selector: 'cd-prometheus-tabs',
  templateUrl: './prometheus-tabs.component.html',
  styleUrls: ['./prometheus-tabs.component.scss']
})
export class PrometheusTabsComponent {
  constructor(public router: Router, public prometheusAlertService: PrometheusAlertService) {}
}
