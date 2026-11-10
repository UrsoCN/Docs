/* Inkspace / Inkspace: resolve the theme before first paint, then wire the control. */
(function () {
  'use strict';

  var storageKey = 'urso-inkspace-theme';
  var root = document.documentElement;
  var system = window.matchMedia('(prefers-color-scheme: dark)');
  var preference = null;
  try {
    var stored = localStorage.getItem(storageKey);
    if (stored === 'light' || stored === 'dark') preference = stored;
  } catch (_) { /* The theme still works when browser storage is unavailable. */ }

  function applyTheme() {
    var dark = (preference || (system.matches ? 'dark' : 'light')) === 'dark';
    root.dataset.theme = dark ? 'dark' : 'light';
    root.style.colorScheme = root.dataset.theme;
    var meta = document.querySelector('meta[name="theme-color"]');
    if (meta) meta.content = dark ? '#171e1c' : '#f6f4ed';
    var button = document.querySelector('.theme-toggle');
    if (button) {
      button.setAttribute('aria-pressed', String(dark));
      button.setAttribute('aria-label', dark ? '切换到日间模式' : '切换到夜间模式');
      button.title = button.getAttribute('aria-label');
      button.querySelector('.theme-toggle-label').textContent = dark ? '夜间' : '日间';
    }
    window.dispatchEvent(new CustomEvent('inkspace:themechange', { detail: { dark: dark } }));
  }

  applyTheme();
  system.addEventListener('change', function () {
    if (!preference) applyTheme();
  });
  window.addEventListener('storage', function (event) {
    if (event.key !== storageKey && event.key !== null) return;
    preference = event.newValue === 'light' || event.newValue === 'dark' ? event.newValue : null;
    applyTheme();
  });
  document.addEventListener('DOMContentLoaded', function () {
    applyTheme();
    document.querySelector('.theme-toggle').addEventListener('click', function () {
      preference = root.dataset.theme === 'dark' ? 'light' : 'dark';
      try { localStorage.setItem(storageKey, preference); } catch (_) {}
      applyTheme();
    });
    var navigationButton = document.querySelector('.inkspace-nav-toggle');
    navigationButton.addEventListener('click', function () {
      var toggle = document.querySelector('.sidebar-toggle-button');
      if (toggle) toggle.click();
    });
    // Docsify mounts its sidebar in another DOMContentLoaded handler.
    requestAnimationFrame(function () {
      var sidebar = document.querySelector('.sidebar');
      if (!sidebar) return;
      function syncNavigation() {
        var open = sidebar.classList.contains('show');
        navigationButton.setAttribute('aria-expanded', String(open));
        navigationButton.setAttribute('aria-label', open ? '收起笔记目录' : '打开笔记目录');
      }
      new MutationObserver(syncNavigation).observe(sidebar, { attributes: true, attributeFilter: ['class'] });
      syncNavigation();
    });
  });
}());
