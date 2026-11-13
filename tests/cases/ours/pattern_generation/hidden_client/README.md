# Hidden standalone client events

Self-constructed variant of the resource-slice model. A common client clocks
both registers while its output mux initially observes only the gate. Each
register captures its own held value, so short scans can change hidden target
storage. Source/generated/flat replay and vendor qualification must preserve
those shared events. Export alone does not establish ordered-apply equivalence.
