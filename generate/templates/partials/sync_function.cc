
{%partial doc .%}
NAN_METHOD({{ cppClassName }}::{{ cppFunctionName }}) {
  NanEscapableScope();
  {%partial guardArguments .%}

  {%each .|returnsInfo 'true' as _return %}
    {%if _return.shouldAlloc %}
  {{ _return.cType }}{{ _return.name }} = ({{ _return.cType }})malloc(sizeof({{ _return.cType|unPointer }}));
    {%else%}
  {{ _return.cType|unPointer }} {{ _return.name }} = {{ _return.cType|unPointer|defaultValue }};
    {%endif%}
  {%endeach%}

  {%each args|argsInfo as arg %}
    {%if not arg.isSelf %}
      {%if not arg.isReturn %}
        {%if not arg.isCallbackFunction %}
          {%if not arg.payloadFor %}
            {%partial convertFromV8 arg %}
          {%endif%}
        {%endif%}
      {%endif%}
    {%endif%}
  {%endeach%}

{%each args|argsInfo as arg %}
  {%if arg.isCallbackFunction %}
NanCallback* {{ arg.name }}_callback = new NanCallback(args[{{ arg.jsArg }}].As<Function>());
  {%endif%}
{%endeach%}
{%if .|hasReturns %}
  {{ return.cType }} result = {%endif%}{{ cFunctionName }}(
  {%each args|argsInfo as arg %}
    {%if arg.isReturn %}
      {%if not arg.shouldAlloc %}&{%endif%}
    {%endif%}
    {%if arg.isSelf %}
ObjectWrap::Unwrap<{{ arg.cppClassName }}>(args.This())->GetValue()
    {%elsif arg.isReturn %}
{{ arg.name }}
    {%elsif arg.isCallbackFunction %}
{{ cppFunctionName }}_{{ arg.name }}_cppCallback,
{{ arg.name }}_callback
    {%elsif arg.payloadFor %}
{%-- payloads are handled inside of the callback condition --%}
    {%else%}
from_{{ arg.name }}
    {%endif%}
    {%if not arg.lastArg %},{%endif%}
  {%endeach%}
  );

{%each args|argsInfo as arg %}
  {%if arg.isCallbackFunction %}
delete {{ arg.name }}_callback;
  {%endif%}
{%endeach%}

{%if return.isErrorCode %}
  if (result != GIT_OK) {
  {%each args|argsInfo as arg %}
    {%if arg.shouldAlloc %}
    free({{ arg.name }});
    {%elsif arg | isOid %}
    if (args[{{ arg.jsArg }}]->IsString()) {
      free({{ arg.name }});
    }
    {%endif%}
  {%endeach%}

    if (giterr_last()) {
      return NanThrowError(giterr_last()->message);
    } else {
      return NanThrowError("Unknown Error");
    }
  }
{%endif%}

{%each args|argsInfo as arg %}
  {%if arg | isOid %}
  if (args[{{ arg.jsArg }}]->IsString()) {
    free(&from_{{ arg.name }});
  }
  {%endif%}
{%endeach%}

{%if not .|returnsCount %}
  NanReturnUndefined();
{%else%}
  {%if return.cType | isPointer %}
  // null checks on pointers
  if (!result) {
    NodeGitPsueodoNanReturnEscapingValue(NanUndefined());
  }
  {%endif%}

  Handle<v8::Value> to;
  {%if .|returnsCount > 1 %}
  Handle<Object> toReturn = NanNew<Object>();
  {%endif%}
  {%each .|returnsInfo as _return %}
    {%partial convertToV8 _return %}
    {%if .|returnsCount > 1 %}
  toReturn->Set(NanNew<String>("{{ _return.returnNameOrName }}"), to);
    {%endif%}
  {%endeach%}
  {%if .|returnsCount == 1 %}
  NodeGitPsueodoNanReturnEscapingValue(to);
  {%else%}
  NodeGitPsueodoNanReturnEscapingValue(toReturn);
  {%endif%}
{%endif%}
}

{%partial callbackHelpers .%}
