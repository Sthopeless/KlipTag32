from esphome import automation, pins
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import (
    CONF_ON_TAG,
    CONF_ON_TAG_REMOVED,
    CONF_RESET_PIN,
    CONF_TRIGGER_ID,
)

CODEOWNERS = ["@glmnet"]
AUTO_LOAD = ["binary_sensor"]

CONF_RC522_ID = "rc522_id"
CONF_ON_NDEF_TEXT = "on_ndef_text"

rc522_ns = cg.esphome_ns.namespace("rc522")
RC522 = rc522_ns.class_("RC522", cg.PollingComponent)
RC522Trigger = rc522_ns.class_(
    "RC522Trigger", automation.Trigger.template(cg.std_string)
)
RC522NdefTextTrigger = rc522_ns.class_(
    "RC522NdefTextTrigger", automation.Trigger.template(cg.std_string)
)

RC522_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(RC522),
        cv.Optional(CONF_RESET_PIN): pins.gpio_output_pin_schema,
        cv.Optional(CONF_ON_TAG): automation.validate_automation(
            {
                cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(RC522Trigger),
            }
        ),
        cv.Optional(CONF_ON_TAG_REMOVED): automation.validate_automation(
            {
                cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(RC522Trigger),
            }
        ),
        cv.Optional(CONF_ON_NDEF_TEXT): automation.validate_automation(
            {
                cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(RC522NdefTextTrigger),
            }
        ),
    }
).extend(cv.polling_component_schema("1s"))


async def setup_rc522(var, config):
    await cg.register_component(var, config)

    if CONF_RESET_PIN in config:
        reset = await cg.gpio_pin_expression(config[CONF_RESET_PIN])
        cg.add(var.set_reset_pin(reset))

    for conf in config.get(CONF_ON_TAG, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID])
        cg.add(var.register_ontag_trigger(trigger))
        await automation.build_automation(trigger, [(cg.std_string, "x")], conf)

    for conf in config.get(CONF_ON_TAG_REMOVED, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID])
        cg.add(var.register_ontagremoved_trigger(trigger))
        await automation.build_automation(trigger, [(cg.std_string, "x")], conf)

    for conf in config.get(CONF_ON_NDEF_TEXT, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID])
        cg.add(var.register_on_ndef_text_trigger(trigger))
        await automation.build_automation(trigger, [(cg.std_string, "x")], conf)
