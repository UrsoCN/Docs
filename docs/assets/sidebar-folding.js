/* Preserve nested navigation folding without the Docsify 4 sidebar plugin. */
(function () {
  'use strict';

  function sidebarFolding(hook) {
    var expanded = new Map();

    hook.doneEach(function () {
      var sidebar = document.querySelector('.sidebar-nav');
      if (!sidebar) return;

      Array.from(sidebar.querySelectorAll('li')).forEach(function (item) {
        var list = item.querySelector(':scope > ul:not(.app-sub-sidebar)');
        if (!list || item.querySelector(':scope > .sidebar-fold-toggle')) return;

        var title = item.querySelector(':scope > a, :scope > p');
        var label = title ? title.textContent.trim() : Array.from(item.childNodes)
          .filter(function (node) { return node.nodeType === Node.TEXT_NODE; })
          .map(function (node) { return node.textContent; }).join('').trim();
        var parent = item.parentElement.closest('li');
        var key = (parent && parent.dataset.foldKey || '') + '/' + label;
        item.dataset.foldKey = key;
        item.classList.add('sidebar-fold-group');

        var button = document.createElement('button');
        button.type = 'button';
        button.className = 'sidebar-fold-toggle';
        var containsActivePage = !!item.querySelector('.active');
        var open = containsActivePage || expanded.get(key) === true;

        function setExpanded(value) {
          list.hidden = !value;
          button.setAttribute('aria-expanded', String(value));
          button.setAttribute('aria-label', (value ? '收起 ' : '展开 ') + label);
        }

        button.addEventListener('click', function (event) {
          event.stopPropagation();
          var value = button.getAttribute('aria-expanded') !== 'true';
          expanded.set(key, value);
          setExpanded(value);
        });
        setExpanded(open);
        item.insertBefore(button, item.firstChild);
      });
    });
  }

  window.$docsify.plugins = (window.$docsify.plugins || []).concat(sidebarFolding);
}());
