/* Render diagrams with the same palette as the reading surface, including live switches. */
(function () {
  'use strict';

  var sources = new WeakMap();
  var queue = Promise.resolve();

  function renderDiagrams() {
    // Mermaid shares configuration globally. Serialize renders during rapid toggles.
    queue = queue.then(async function () {
      if (!window.mermaid) return;
      var nodes = Array.from(document.querySelectorAll('#main .mermaid'));
      if (!nodes.length) return;
      var dark = document.documentElement.dataset.theme === 'dark';
      window.mermaid.initialize({
        startOnLoad: false,
        theme: 'base',
        themeVariables: {
          darkMode: dark,
          background: dark ? '#1d2723' : '#efeee6',
          primaryColor: dark ? '#273f35' : '#dfeadf',
          primaryTextColor: dark ? '#dfe8de' : '#263a30',
          primaryBorderColor: dark ? '#8bb89c' : '#567a64',
          secondaryColor: dark ? '#393729' : '#f0e6cb',
          tertiaryColor: dark ? '#263a3e' : '#e0e9eb',
          lineColor: dark ? '#adbdae' : '#5a7061',
          textColor: dark ? '#dfe8de' : '#263a30',
          fontFamily: 'system-ui, "Noto Sans CJK SC", sans-serif',
          fontSize: '14px'
        }
      });
      nodes.forEach(function (node) {
        if (!sources.has(node)) sources.set(node, node.textContent);
        node.removeAttribute('data-processed');
        node.textContent = sources.get(node);
      });
      await window.mermaid.run({ nodes: nodes });
    }).catch(function (error) {
      console.error('Mermaid rendering failed:', error);
    });
  }

  window.addEventListener('inkspace:themechange', renderDiagrams);
  window.$docsify.plugins = (window.$docsify.plugins || []).concat(function (hook) {
    hook.afterEach(function (html) {
      var template = document.createElement('template');
      template.innerHTML = html;
      template.content.querySelectorAll('pre[data-lang="mermaid"]').forEach(function (pre) {
        var diagram = document.createElement('div');
        diagram.className = 'mermaid';
        diagram.textContent = pre.textContent;
        pre.replaceWith(diagram);
      });
      return template.innerHTML;
    });
    hook.doneEach(renderDiagrams);
  });
}());
