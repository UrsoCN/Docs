/* Render Obsidian callout Markdown after Markdown parsing, preserving its content. */
(function () {
  'use strict';

  var aliases = {
    note: 'note', abstract: 'info', summary: 'info', tldr: 'info',
    info: 'info', todo: 'info', tip: 'tip', hint: 'tip', important: 'tip',
    success: 'success', check: 'success', done: 'success',
    question: 'question', help: 'question', faq: 'question',
    warning: 'warning', caution: 'warning', attention: 'warning',
    failure: 'danger', fail: 'danger', missing: 'danger',
    danger: 'danger', error: 'danger', bug: 'danger',
    example: 'example', quote: 'quote', cite: 'quote'
  };

  function calloutPlugin(hook) {
    hook.afterEach(function (html) {
      var template = document.createElement('template');
      template.innerHTML = html;

      // Children first, so nested callouts survive replacing their parent quote.
      Array.from(template.content.querySelectorAll('blockquote')).reverse().forEach(function (quote) {
        var first = quote.firstElementChild;
        if (!first || first.tagName !== 'P') return;

        var match = first.innerHTML.match(/^\s*\[!([a-z0-9-]+)\]([+-])?[ \t]*([^\n]*?)(?:\r?\n|<br\s*\/?>|$)/i);
        if (!match) return;

        var type = match[1].toLowerCase();
        var fold = match[2];
        var box = document.createElement(fold ? 'details' : 'aside');
        box.className = 'obsidian-callout';
        box.dataset.callout = type;
        box.dataset.calloutKind = aliases[type] || 'note';
        if (fold === '+') box.open = true;

        var title = document.createElement(fold ? 'summary' : 'div');
        title.className = 'obsidian-callout-title';
        title.innerHTML = match[3].trim() || type.charAt(0).toUpperCase() + type.slice(1);
        box.appendChild(title);

        first.innerHTML = first.innerHTML.slice(match[0].length);
        if (!first.innerHTML.trim()) first.remove();

        var body = document.createElement('div');
        body.className = 'obsidian-callout-body';
        while (quote.firstChild) body.appendChild(quote.firstChild);
        box.appendChild(body);
        quote.replaceWith(box);
      });

      return template.innerHTML;
    });
  }

  window.$docsify = window.$docsify || {};
  window.$docsify.plugins = (window.$docsify.plugins || []).concat(calloutPlugin);
}());
